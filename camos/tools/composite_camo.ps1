param(
    [Parameter(Mandatory=$true)][string]$BaseTexture,
    [string]$CamoDir,
    [Parameter(Mandatory=$true)][string]$OutDir,
    [Parameter(Mandatory=$true)][string]$OutPrefix,
    [switch]$Bake,
    [int]$NumCamos = 8,
    [double]$WoodHueMin = 8,
    [double]$WoodHueMax = 50,
    [double]$WoodSatMin = 0.22,
    [double]$WoodValMin = 0.02,
    [double]$MaskBlurRadius = 5,
    [double]$DetailBaseline = 40.0,
    [double]$DetailGain = 2.5
)

# Default mode (re)builds <OutPrefix>_mask.png: ALPHA=paint mask, RGB=detail/AO multiplier re-centered on this weapon's own paintable-area average luma, clamped +-40/255 (128=neutral, maskMap modulates pattern by this x2).
# Pass -Bake (and -CamoDir) to also pre-composite the old <OutPrefix>_camoN.jpg fallback per pattern.

Add-Type -AssemblyName System.Drawing

$src = @"
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

public static class CamoCompositor
{
    public static byte[] GetPixels(Bitmap bmp, out int stride)
    {
        Rectangle rect = new Rectangle(0, 0, bmp.Width, bmp.Height);
        BitmapData bd = bmp.LockBits(rect, ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
        stride = bd.Stride;
        int bytes = bd.Stride * bmp.Height;
        byte[] data = new byte[bytes];
        Marshal.Copy(bd.Scan0, data, 0, bytes);
        bmp.UnlockBits(bd);
        return data;
    }

    public static void SetPixels(Bitmap bmp, byte[] data, int stride)
    {
        Rectangle rect = new Rectangle(0, 0, bmp.Width, bmp.Height);
        BitmapData bd = bmp.LockBits(rect, ImageLockMode.WriteOnly, PixelFormat.Format32bppArgb);
        Marshal.Copy(data, 0, bd.Scan0, data.Length);
        bmp.UnlockBits(bd);
    }

    static void RgbToHsv(byte r, byte g, byte b, out double h, out double s, out double v)
    {
        double rr = r / 255.0, gg = g / 255.0, bb = b / 255.0;
        double max = Math.Max(rr, Math.Max(gg, bb));
        double min = Math.Min(rr, Math.Min(gg, bb));
        double delta = max - min;
        v = max;
        s = (max <= 0.0001) ? 0 : delta / max;
        h = 0;
        if (delta > 0.0001)
        {
            if (max == rr) h = 60.0 * (((gg - bb) / delta) % 6.0);
            else if (max == gg) h = 60.0 * (((bb - rr) / delta) + 2.0);
            else h = 60.0 * (((rr - gg) / delta) + 4.0);
            if (h < 0) h += 360.0;
        }
    }

    // Builds a 0..255 paint mask: 255 = paintable (metal/other), 0 = protected (wood/grip)
    public static byte[] BuildMask(byte[] basePix, int w, int h, int stride, double hueMin, double hueMax, double satMin, double valMin, int blurRadius)
    {
        byte[] mask = new byte[w * h];
        for (int y = 0; y < h; y++)
        {
            int row = y * stride;
            for (int x = 0; x < w; x++)
            {
                int idx = row + x * 4;
                byte b = basePix[idx + 0];
                byte g = basePix[idx + 1];
                byte r = basePix[idx + 2];
                double hh, ss, vv;
                RgbToHsv(r, g, b, out hh, out ss, out vv);
                bool isWood = (hh >= hueMin && hh <= hueMax && ss >= satMin && vv > valMin);
                mask[y * w + x] = (byte)(isWood ? 0 : 255);
            }
        }

        // separable box blur, a few passes, to feather the binary edges
        for (int pass = 0; pass < 3; pass++)
        {
            mask = BoxBlurH(mask, w, h, blurRadius);
            mask = BoxBlurV(mask, w, h, blurRadius);
        }
        return mask;
    }

    // Detail/AO multiplier (maskMap modulates pattern by this x2, 128=unchanged): re-centered on this weapon's own paint-mask-weighted average luma so uniformly dark base textures don't crush the pattern, deviation clamped +-40/255.
    public static byte[] BuildDetail(byte[] basePix, int w, int h, int stride, byte[] mask, double DetailBaseline, double DetailGain)
    {
        double weightedSum = 0.0, weightSum = 0.0;
        for (int y = 0; y < h; y++)
        {
            int row = y * stride;
            for (int x = 0; x < w; x++)
            {
                int idx = row + x * 4;
                byte b = basePix[idx + 0];
                byte g = basePix[idx + 1];
                byte r = basePix[idx + 2];
                double luma = 0.299 * r + 0.587 * g + 0.114 * b;
                double weight = mask[y * w + x] / 255.0;
                weightedSum += luma * weight;
                weightSum += weight;
            }
        }
        double avgLuma = (weightSum > 0.0001) ? (weightedSum / weightSum) : 128.0;

        byte[] detail = new byte[w * h];
        for (int y = 0; y < h; y++)
        {
            int row = y * stride;
            for (int x = 0; x < w; x++)
            {
                int idx = row + x * 4;
                byte b = basePix[idx + 0];
                byte g = basePix[idx + 1];
                byte r = basePix[idx + 2];
                double luma = 0.299 * r + 0.587 * g + 0.114 * b;
                double centered = Clamp((int)Math.Round((luma - avgLuma) * DetailGain), -40, 40);
                detail[y * w + x] = (byte)Clamp((int)(DetailBaseline + centered + 0.5), 0, 255);
            }
        }
        return detail;
    }

    static byte[] BoxBlurH(byte[] src, int w, int h, int radius)
    {
        byte[] dst = new byte[w * h];
        for (int y = 0; y < h; y++)
        {
            int row = y * w;
            int sum = 0;
            int count = 0;
            for (int x = -radius; x <= radius; x++)
            {
                int xx = Clamp(x, 0, w - 1);
                sum += src[row + xx];
                count++;
            }
            for (int x = 0; x < w; x++)
            {
                dst[row + x] = (byte)(sum / count);
                int xAdd = Clamp(x + radius + 1, 0, w - 1);
                int xSub = Clamp(x - radius, 0, w - 1);
                sum += src[row + xAdd] - src[row + xSub];
            }
        }
        return dst;
    }

    static byte[] BoxBlurV(byte[] src, int w, int h, int radius)
    {
        byte[] dst = new byte[w * h];
        for (int x = 0; x < w; x++)
        {
            int sum = 0;
            int count = 0;
            for (int y = -radius; y <= radius; y++)
            {
                int yy = Clamp(y, 0, h - 1);
                sum += src[yy * w + x];
                count++;
            }
            for (int y = 0; y < h; y++)
            {
                dst[y * w + x] = (byte)(sum / count);
                int yAdd = Clamp(y + radius + 1, 0, h - 1);
                int ySub = Clamp(y - radius, 0, h - 1);
                sum += src[yAdd * w + x] - src[ySub * w + x];
            }
        }
        return dst;
    }

    static int Clamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

    static double Overlay(double b, double c)
    {
        return (b < 0.5) ? (2.0 * b * c) : (1.0 - 2.0 * (1.0 - b) * (1.0 - c));
    }

    public static Bitmap Composite(Bitmap baseBmp, Bitmap camoBmp, byte[] mask, double camoStrength)
    {
        int w = baseBmp.Width, h = baseBmp.Height;

        Bitmap camoResized = camoBmp;
        bool disposeCamo = false;
        if (camoBmp.Width != w || camoBmp.Height != h)
        {
            camoResized = new Bitmap(w, h, PixelFormat.Format32bppArgb);
            using (Graphics gr = Graphics.FromImage(camoResized))
            {
                gr.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic;
                gr.DrawImage(camoBmp, 0, 0, w, h);
            }
            disposeCamo = true;
        }

        int baseStride, camoStride;
        byte[] basePix = GetPixels(baseBmp, out baseStride);
        byte[] camoPix = GetPixels(camoResized, out camoStride);

        Bitmap outBmp = new Bitmap(w, h, PixelFormat.Format32bppArgb);
        byte[] outPix = new byte[basePix.Length];

        for (int y = 0; y < h; y++)
        {
            int baseRow = y * baseStride;
            int camoRow = y * camoStride;
            for (int x = 0; x < w; x++)
            {
                int bi = baseRow + x * 4;
                int ci = camoRow + x * 4;

                byte bb = basePix[bi + 0];
                byte bg = basePix[bi + 1];
                byte br = basePix[bi + 2];

                byte cb = camoPix[ci + 0];
                byte cg = camoPix[ci + 1];
                byte cr = camoPix[ci + 2];

                double luma = (0.299 * br + 0.587 * bg + 0.114 * bb) / 255.0;

                double ovR = Overlay(luma, cr / 255.0);
                double ovG = Overlay(luma, cg / 255.0);
                double ovB = Overlay(luma, cb / 255.0);

                double maskWeight = (mask[y * w + x] / 255.0) * camoStrength;

                double finalR = br / 255.0 * (1.0 - maskWeight) + ovR * maskWeight;
                double finalG = bg / 255.0 * (1.0 - maskWeight) + ovG * maskWeight;
                double finalB = bb / 255.0 * (1.0 - maskWeight) + ovB * maskWeight;

                outPix[bi + 0] = (byte)Clamp((int)(finalB * 255.0 + 0.5), 0, 255);
                outPix[bi + 1] = (byte)Clamp((int)(finalG * 255.0 + 0.5), 0, 255);
                outPix[bi + 2] = (byte)Clamp((int)(finalR * 255.0 + 0.5), 0, 255);
                outPix[bi + 3] = 255;
            }
        }

        SetPixels(outBmp, outPix, baseStride);
        if (disposeCamo) camoResized.Dispose();
        return outBmp;
    }

    public static Bitmap MaskToBitmap(byte[] mask, byte[] detail, int w, int h)
    {
        Bitmap bmp = new Bitmap(w, h, PixelFormat.Format32bppArgb);
        byte[] pix = new byte[w * h * 4];
        for (int i = 0; i < w * h; i++)
        {
            byte d = detail[i];
            pix[i * 4 + 0] = d;
            pix[i * 4 + 1] = d;
            pix[i * 4 + 2] = d;
            pix[i * 4 + 3] = mask[i];
        }
        int stride;
        Rectangle rect = new Rectangle(0, 0, w, h);
        BitmapData bd = bmp.LockBits(rect, ImageLockMode.WriteOnly, PixelFormat.Format32bppArgb);
        stride = bd.Stride;
        // handle stride padding
        byte[] padded = new byte[stride * h];
        for (int y = 0; y < h; y++)
            Array.Copy(pix, y * w * 4, padded, y * stride, w * 4);
        Marshal.Copy(padded, 0, bd.Scan0, padded.Length);
        bmp.UnlockBits(bd);
        return bmp;
    }
}
"@

Add-Type -TypeDefinition $src -ReferencedAssemblies System.Drawing

function Save-Jpeg($bitmap, $path, $quality) {
    $codec = [System.Drawing.Imaging.ImageCodecInfo]::GetImageEncoders() | Where-Object { $_.MimeType -eq 'image/jpeg' }
    $params = New-Object System.Drawing.Imaging.EncoderParameters(1)
    $params.Param[0] = New-Object System.Drawing.Imaging.EncoderParameter([System.Drawing.Imaging.Encoder]::Quality, [long]$quality)
    $bitmap.Save($path, $codec, $params)
}

Write-Output "Loading base texture: $BaseTexture"
$baseBmp = [System.Drawing.Bitmap]::FromFile($BaseTexture)
$baseBmp32 = New-Object System.Drawing.Bitmap($baseBmp.Width, $baseBmp.Height, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
$g = [System.Drawing.Graphics]::FromImage($baseBmp32)
$g.DrawImage($baseBmp, 0, 0, $baseBmp.Width, $baseBmp.Height)
$g.Dispose()
$baseBmp.Dispose()

Write-Output "Building paint mask ($($baseBmp32.Width)x$($baseBmp32.Height))..."
$stride = 0
$basePixels = [CamoCompositor]::GetPixels($baseBmp32, [ref]$stride)
$mask = [CamoCompositor]::BuildMask($basePixels, $baseBmp32.Width, $baseBmp32.Height, $stride, $WoodHueMin, $WoodHueMax, $WoodSatMin, $WoodValMin, [int]$MaskBlurRadius)
$detail = [CamoCompositor]::BuildDetail($basePixels, $baseBmp32.Width, $baseBmp32.Height, $stride, $mask, $DetailBaseline, $DetailGain)

if (!(Test-Path $OutDir)) { New-Item -ItemType Directory -Path $OutDir -Force | Out-Null }

$maskBmp = [CamoCompositor]::MaskToBitmap($mask, $detail, $baseBmp32.Width, $baseBmp32.Height)
$maskPath = Join-Path $OutDir "$($OutPrefix)_mask.png"
$maskBmp.Save($maskPath, [System.Drawing.Imaging.ImageFormat]::Png)
$maskBmp.Dispose()
Write-Output "Saved mask: $maskPath"

if ($Bake) {
    if (!$CamoDir) {
        Write-Warning "-Bake requires -CamoDir, skipping bake step"
    } else {
        for ($i = 1; $i -le $NumCamos; $i++) {
            $camoFile = Join-Path $CamoDir ("camouflage_{0:D2}.png" -f $i)
            if (!(Test-Path $camoFile)) {
                Write-Warning "Missing camo pattern: $camoFile - skipping"
                continue
            }
            Write-Output "Compositing camo $i from $camoFile"
            $camoBmp = [System.Drawing.Bitmap]::FromFile($camoFile)
            $camoBmp32 = New-Object System.Drawing.Bitmap($camoBmp.Width, $camoBmp.Height, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
            $g2 = [System.Drawing.Graphics]::FromImage($camoBmp32)
            $g2.DrawImage($camoBmp, 0, 0, $camoBmp.Width, $camoBmp.Height)
            $g2.Dispose()
            $camoBmp.Dispose()

            $resultBmp = [CamoCompositor]::Composite($baseBmp32, $camoBmp32, $mask, 1.0)
            $outPath = Join-Path $OutDir "$($OutPrefix)_camo$i.jpg"
            Save-Jpeg $resultBmp $outPath 92
            $resultBmp.Dispose()
            $camoBmp32.Dispose()
            Write-Output "  -> $outPath"
        }
    }
}

$baseBmp32.Dispose()
Write-Output "Done."
