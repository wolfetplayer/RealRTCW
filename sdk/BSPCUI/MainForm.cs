using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Windows.Forms;

namespace BSPCUI
{
    public partial class MainForm : Form
    {
        private string m_sessionName = "last_session.ini";

        private string RemoveSpace(string str)
        {
            int pos;

            while ((pos = str.IndexOf(' ')) >= 0)
            {
                str = str.Remove(pos, 1);
            }

            return str;
        }

        private bool RecognizeValue(string val)
        {
            return val == "1" || val == "yes" || val == "+" || val == "on" ||
                val == "enabled" || val == "true";
        }

        private void ListString2PairKeyValue(string[] pair, out string key, out string val)
        {
            key = pair[0];
            val = pair[1];
        }

        private bool LoadConfig()
        {
            try
            {
                StreamReader sr = new StreamReader(m_sessionName);
                string line, key, val;

                while ((line = sr.ReadLine()) != null)
                {
                    line = RemoveSpace(line.ToLower());

                    ListString2PairKeyValue(line.Split('='), out key, out val);

                    if (key == "optimize")
                    {
                        chkb_optimize.Checked = RecognizeValue(val);
                    }
                    else if (key == "neverbose")
                    {
                        chkb_neverbose.Checked = RecognizeValue(val);
                    }
                    else if (key == "breadthfirst")
                    {
                        chkb_breadthfirst.Checked = RecognizeValue(val);
                    }
                    else if (key == "nobrushmerge")
                    {
                        chkb_nobrushmerge.Checked = RecognizeValue(val);
                    }
                    else if (key == "noliquids")
                    {
                        chkb_noliquids.Checked = RecognizeValue(val);
                    }
                    else if (key == "freetree")
                    {
                        chkb_freetree.Checked = RecognizeValue(val);
                    }
                    else if (key == "nocsg")
                    {
                        chkb_nocsg.Checked = RecognizeValue(val);
                    }
                    else if (key == "forcesidesvisible")
                    {
                        chkb_forcesidesvisible.Checked = RecognizeValue(val);
                    }
                    else if (key == "grapplereach")
                    {
                        chkb_grapplereach.Checked = RecognizeValue(val);
                    }
                    else if (key == "writeaasmap")
                    {
                        chkb_writeaasmap.Checked = RecognizeValue(val);
                    }
                    else if (key == "cfg1switch")
                    {
                        chkb_cfg1.Checked = RecognizeValue(val);
                    }
                    else if (key == "cfg2switch")
                    {
                        chkb_cfg2.Checked = RecognizeValue(val);
                    }
                    else if (key == "threads")
                    {
                        counter_threads.Value = int.Parse(val);
                    }
                    else if (key == "bsp2aas")
                    {
                        txtb_bsp2aas.Text = val;
                    }
                    else if (key == "output")
                    {
                        txtb_output.Text = val;
                    }
                    else if (key == "cfg1")
                    {
                        txtb_cfg1.Text = val;
                    }
                    else if (key == "cfg2")
                    {
                        txtb_cfg2.Text = val;
                    }

                };

                sr.Close();
            }
            catch
            {
                return false;
            }
            finally
            {}

            return true;
        }

        private bool SaveConfig()
        {
            try
            {
                StreamWriter sw = new StreamWriter(m_sessionName);

                sw.WriteLine(string.Format("optimize = {0}", chkb_optimize.Checked));
                sw.WriteLine(string.Format("neverbose = {0}", chkb_neverbose.Checked));
                sw.WriteLine(string.Format("breadthfirst = {0}", chkb_breadthfirst.Checked));
                sw.WriteLine(string.Format("nobrushmerge = {0}", chkb_nobrushmerge.Checked));
                sw.WriteLine(string.Format("noliquids = {0}", chkb_noliquids.Checked));
                sw.WriteLine(string.Format("freetree = {0}", chkb_freetree.Checked));
                sw.WriteLine(string.Format("nocsg = {0}", chkb_nocsg.Checked));
                sw.WriteLine(string.Format("forcesidesvisible = {0}", chkb_forcesidesvisible.Checked));
                sw.WriteLine(string.Format("grapplereach = {0}", chkb_grapplereach.Checked));
                sw.WriteLine(string.Format("writeaasmap = {0}", chkb_writeaasmap.Checked));
                sw.WriteLine(string.Format("cfg1switch = {0}", chkb_cfg1.Checked));
                sw.WriteLine(string.Format("cfg2switch = {0}", chkb_cfg2.Checked));
                sw.WriteLine(string.Format("threads = {0}", counter_threads.Value));
                sw.WriteLine(string.Format("bsp2aas = {0}", txtb_bsp2aas.Text));
                sw.WriteLine(string.Format("output = {0}", txtb_output.Text));
                sw.WriteLine(string.Format("cfg1 = {0}", txtb_cfg1.Text));
                sw.WriteLine(string.Format("cfg2 = {0}", txtb_cfg2.Text));

                sw.Close();
            }
            catch
            {
                return false;
            }
            finally
            { }

            return true;
        }

        private bool TouchConfig()
        {
            try
            {
                StreamWriter sw = new StreamWriter(m_sessionName);

                sw.WriteLine("optimize = false");
                sw.WriteLine("neverbose = false");
                sw.WriteLine("breadthfirst = false");
                sw.WriteLine("nobrushmerge = false");
                sw.WriteLine("noliquids = false");
                sw.WriteLine("freetree = false");
                sw.WriteLine("nocsg = false");
                sw.WriteLine("forcesidesvisible = false");
                sw.WriteLine("grapplereach = false");
                sw.WriteLine("writeaasmap = false");
                sw.WriteLine("cfg1switch = true");
                sw.WriteLine("cfg2switch = false");
                sw.WriteLine("threads = 2");
                sw.WriteLine("bsp2aas =");
                sw.WriteLine("output =");
                sw.WriteLine("cfg1 =");
                sw.WriteLine("cfg2 =");

                sw.Close();
            }
            catch
            {
                return false;
            }
            finally
            {}

            return true;
        }

        private string ParseCheckBox(CheckBox checkbox)
        {
            return checkbox.Checked ? '-' + checkbox.Text + ' ' : "";
        }

        public MainForm()
        {
            InitializeComponent();

            if (!LoadConfig())
            {
                if (!TouchConfig())
                {
                    throw new Exception("Can't create default config");
                }

                if (!LoadConfig())
                {
                    throw new Exception("Can't load default config");
                }
            }
        }

        private void bsp2aas_Click(object sender, EventArgs e)
        {
            OpenFileDialog bsp = new OpenFileDialog
            {
                Filter = "bsp file (*.bsp)|*.bsp",
                FileName = txtb_bsp2aas.Text
            };

            if (bsp.ShowDialog() == DialogResult.OK)
            {
                txtb_bsp2aas.Text = bsp.FileName;
            }
        }

        private void output_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog aasdir = new FolderBrowserDialog
            {
                ShowNewFolderButton = false,
                SelectedPath = txtb_output.Text
            };

            if (aasdir.ShowDialog() == DialogResult.OK)
            {
                txtb_output.Text = aasdir.SelectedPath;
            }
        }

        private void cfg1_Click(object sender, EventArgs e)
        {
            OpenFileDialog cfg = new OpenFileDialog
            {
                Filter = "cfg file (*.c)|*.c",
                FileName = txtb_cfg1.Text
            };

            if (cfg.ShowDialog() == DialogResult.OK)
            {
                txtb_cfg1.Text = cfg.FileName;
            }
        }

        private void cfg2_Click(object sender, EventArgs e)
        {
            OpenFileDialog cfg = new OpenFileDialog
            {
                Filter = "cfg file (*.c)|*.c",
                FileName = txtb_cfg2.Text
            };

            if (cfg.ShowDialog() == DialogResult.OK)
            {
                txtb_cfg2.Text = cfg.FileName;
            }
        }

        private string AppendCfgParams(string cmdLineArgs, TextBox cfg)
        {
            cmdLineArgs += string.Format("-cfg {0} ", cfg.Text);

            if (cfg.Text.Contains("_sm"))
            {
                cmdLineArgs += string.Format("-ext _b0 ");
            }
            else if (cfg.Text.Contains("_lg"))
            {
                cmdLineArgs += string.Format("-ext _b1 ");
            }

            return cmdLineArgs;
        }

        private Queue<string> CollectAllBspFiles(string path)
        {
            Queue<string> files;

            try
            {
                files = new Queue<string>(Directory.GetFiles(path, "*.bsp"));
            }
            catch (IOException ex)
            {
                files = new Queue<string>();
                files.Enqueue(path);
            }

            return files;
        }

        private void run_Click(object sender, EventArgs e)
        {
            string cmdLineArgs = string.Empty;

            if (!File.Exists("bspc.exe"))
            {
                var res = MessageBox.Show("bspc.exe does not exist", "Error", MessageBoxButtons.OK);

                Application.Exit();
            }

            if (txtb_bsp2aas.TextLength == 0)
            {
                MessageBox.Show("No bsp input file", "Error", MessageBoxButtons.OK);

                return;
            }

            if (txtb_cfg1.TextLength == 0 && txtb_cfg2.TextLength == 0 || 
                !(chkb_cfg1.Checked || chkb_cfg2.Checked))
            {
                MessageBox.Show("No config files", "Error", MessageBoxButtons.OK);

                return;
            }

            if (txtb_output.TextLength > 0)
            {
                cmdLineArgs += string.Format("-output {0}\\ ", txtb_output.Text);
            }
            else
            {
                // it seems that in bspc bug which forms the wrong file extension
                // if you do not specify the output parameter
                cmdLineArgs += "-output .\\ ";
            }

            cmdLineArgs += ParseCheckBox(chkb_breadthfirst);
            cmdLineArgs += ParseCheckBox(chkb_neverbose);
            cmdLineArgs += ParseCheckBox(chkb_optimize);
            cmdLineArgs += ParseCheckBox(chkb_nobrushmerge);
            cmdLineArgs += ParseCheckBox(chkb_noliquids);
            cmdLineArgs += ParseCheckBox(chkb_writeaasmap);
            cmdLineArgs += ParseCheckBox(chkb_grapplereach);
            cmdLineArgs += ParseCheckBox(chkb_freetree);
            cmdLineArgs += ParseCheckBox(chkb_nocsg);
            cmdLineArgs += ParseCheckBox(chkb_forcesidesvisible);

            cmdLineArgs += string.Format("-threads {0} ", counter_threads.Value);

            foreach (string file in CollectAllBspFiles(txtb_bsp2aas.Text))
            {
                cmdLineArgs += string.Format("-bsp2aas {0} ", file);

                if (chkb_cfg1.Checked && txtb_cfg1.TextLength > 0)
                {
                    Process.Start("bspc.exe", AppendCfgParams(cmdLineArgs, txtb_cfg1)).WaitForExit();
                }

                if (chkb_cfg2.Checked && txtb_cfg2.TextLength > 0)
                {
                    Process.Start("bspc.exe", AppendCfgParams(cmdLineArgs, txtb_cfg2)).WaitForExit();
                }
            }
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            SaveConfig();
        }

        private void chkb_cfg1_CheckedChanged(object sender, EventArgs e)
        {
            txtb_cfg1.Enabled = !txtb_cfg1.Enabled;
            btn_cfg1.Enabled = !btn_cfg1.Enabled;
        }

        private void chkb_cfg2_CheckedChanged(object sender, EventArgs e)
        {
            txtb_cfg2.Enabled = !txtb_cfg2.Enabled;
            btn_cfg2.Enabled = !btn_cfg2.Enabled;
        }
    }
}
