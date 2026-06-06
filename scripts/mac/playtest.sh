#!/usr/bin/env bash
# RealRTCW playtest launcher with ASAN/UBSAN capture.
#
# Запускает текущий arm64 билд, ловит любые ASAN/UBSAN reports в /tmp,
# и в конце выводит чеклист, что проверять для 4 фиксов на этой ветке.
#
# Если билд собран с -fsanitize (наличие Makefile.local с -fsanitize=...),
# инструментация активна — играй спокойно, при любом UAF/UB будет вопль
# в stderr ИЛИ в /tmp/rtcw-asan.<pid>.log.
#
# Если хочешь обычный (быстрый, без оверхеда) билд:
#   rm Makefile.local
#   make ARCH=arm64 USE_INTERNAL_LIBS=0 USE_OPENAL=0 -j8
#
# Если хочешь снова ASAN — Makefile.local восстанавливается одной строкой:
#   echo "BASE_CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1\nLDFLAGS += -fsanitize=address,undefined" > Makefile.local
#   make clean && make ARCH=arm64 USE_INTERNAL_LIBS=0 USE_OPENAL=0 -j8

set -uo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BIN="$REPO_ROOT/build/release-darwin-arm64-nosteam/RealRTCW.arm64"
ASAN_LOG_PREFIX="/tmp/rtcw-asan"
UBSAN_LOG_PREFIX="/tmp/rtcw-ubsan"

if [ ! -x "$BIN" ]; then
  echo "ERROR: бинарник не найден: $BIN"
  echo "       собери его: cd $REPO_ROOT && make ARCH=arm64 USE_INTERNAL_LIBS=0 USE_OPENAL=0 -j8"
  exit 1
fi

# Чистим прошлые логи sanitizer'а — чтобы видеть свежие.
rm -f "${ASAN_LOG_PREFIX}".* "${UBSAN_LOG_PREFIX}".* 2>/dev/null

# Определяем, есть ли ASAN в бинаре, чтобы предупредить
if otool -L "$BIN" 2>/dev/null | grep -q asan; then
  echo "═══ RealRTCW ASAN/UBSAN build ═══"
  echo "Sanitizer активен — будет медленнее обычного, но любой UAF/UB будет пойман."
else
  echo "═══ RealRTCW release build (без sanitizer'а) ═══"
fi
echo "Лог-префиксы:  ${ASAN_LOG_PREFIX}.<pid>.log   ${UBSAN_LOG_PREFIX}.<pid>.log"
echo "Бинарник:      $BIN"
echo ""

export ASAN_OPTIONS="abort_on_error=0:detect_leaks=0:halt_on_error=0:log_path=${ASAN_LOG_PREFIX}"
export UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=0:log_path=${UBSAN_LOG_PREFIX}"

# cd в каталог с бинарником, чтобы он нашёл renderer .dylib рядом.
cd "$(dirname "$BIN")"
# stdin → /dev/null: иначе Q3-движок включает встроенную TTY-консоль и
# крадёт клавиатуру у SDL-окна (символы дублируются в терминал).
"$BIN" "$@" < /dev/null
GAME_EXIT=$?

echo ""
echo "═══ Игра вышла с кодом $GAME_EXIT ═══"

ASAN_HITS=$(ls "${ASAN_LOG_PREFIX}".* 2>/dev/null | wc -l | tr -d ' ')
UBSAN_HITS=$(ls "${UBSAN_LOG_PREFIX}".* 2>/dev/null | wc -l | tr -d ' ')

if [ "$ASAN_HITS" -gt 0 ] || [ "$UBSAN_HITS" -gt 0 ]; then
  echo ""
  echo "⚠  Sanitizer что-то нашёл:"
  for f in ${ASAN_LOG_PREFIX}.* ${UBSAN_LOG_PREFIX}.*; do
    [ -f "$f" ] || continue
    echo "─── $f ───"
    head -40 "$f"
    echo "(см. полностью: cat $f)"
    echo ""
  done
else
  echo "✓ Никаких ASAN/UBSAN репортов. Sanitizer молчал — это хорошо."
fi

cat <<'EOF'

═══ ЧЕКЛИСТ — что проверить для 4 фиксов ═══

A1 — low-ammo warning для поздних оружий (cg_playerstate.c)
  • Новая игра / любой сохраняшка
  • В консоли: \cheats 1; \giveall
  • Возьми MP44 / MG42M / BAR / WP_VENOM / WP_PANZERFAUST
    (любое оружие с enum >= 32 — см. bg_public.h:528-599)
  • Постреляй до низкого боезапаса
  • ОЖИДАЕМО: должен прозвучать warning beep + HUD ammo flash
  • БЫЛО ДО ФИКСА: тишина — этот код пути для оружия >=32 был сломан

A2 — savegame UAF (ui_shared.c)
  • Запусти кампанию, дойди до места, сделай Quick Save (F5)
  • ESC → Load Game
  • Наведи курсор на сохраняшку
  • ОЖИДАЕМО: справа корректная инфа — date, level, playtime
  • БЫЛО ДО ФИКСА: каждый кадр после первого item->text dangling — мог
    показываться garbage / crash на ASAN-сборке

A3 — Com_StringContains size_t wrap (common.c)
  • В игре, в консоли: \find этооченьдлиннаяподстрокачтобывсёполомать
  • ОЖИДАЕМО: ничего не находит, не крашит, не подвисает
  • БЫЛО ДО ФИКСА: на arm64 (64→32-bit truncation) — UB, потенциально
    garbage iter / OOB read

A4 — snd_mix NULL chunk wrap (snd_mix.c)
  • Самый сложный для ручной проверки.
  • В консоли: \s_useOpenAL 0; \snd_restart
    (откатываемся на legacy SDL mixer — путь, где жил баг)
  • Поиграй 60+ секунд, послушай звуки
  • ОЖИДАЕМО: звук работает корректно, не крашит на длинных сэмплах
  • БЫЛО ДО ФИКСА: NULL deref на границе chunk'а в SetVoiceAmplitudeFrom16 /
    S_PaintChannelFrom16_scalar / S_PaintChannelFromMuLaw

EOF
