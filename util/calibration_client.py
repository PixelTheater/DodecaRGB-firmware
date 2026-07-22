#!/usr/bin/env python3
"""Interactive DodecaRGB calibration client (WASD) over USB serial NDJSON."""

from __future__ import annotations

import argparse
import json
import sys
import termios
import time
import tty
from typing import Any, Optional

try:
    import serial
    from serial.tools import list_ports
except ImportError as exc:
    raise SystemExit("pyserial required: pip install pyserial") from exc


HELP = """
Keys (device must show Identify Sides):
  A / D     rotate focus CCW / CW (outside view)
  W / S     move focused side up / down the wiring list (swap with next/prev)
  Enter     confirm focus and advance
  [ / ]     previous / next focus wiring index
  R         reset to factory map
  G         print wiring map (table)
  I         begin interactive mode
  H / ?     help
  Q / Esc   quit
"""


def format_focus(result: dict[str, Any]) -> str:
    if not result:
        return "(no focus data)"
    return (
        f"focus w{result.get('focusWiring', '?')}  "
        f"label={result.get('boardLabel', '?')}  "
        f"slot={result.get('slotIndex', '?')}  "
        f"rot={result.get('rotationStep', '?')}  "
        f"resolved={result.get('resolved', '?')}/12"
    )


def format_map(result: dict[str, Any]) -> str:
    rows = result.get("assignments") or []
    focus = result.get("focusWiring")
    lines = [
        f"focus w{focus}  resolved {result.get('resolved', '?')}/12",
        "  w  label  slot  rot",
        "  -- -----  ----  ---",
    ]
    for row in rows:
        mark = ">" if row.get("w") == focus else " "
        lines.append(
            f"{mark} {row.get('w'):>2}   {row.get('boardLabel'):>3}   "
            f"{row.get('slotIndex'):>3}   {row.get('rotationStep'):>2}"
        )
    return "\n".join(lines)


class Client:
    def __init__(self, port: str, baud: int = 115200) -> None:
        self.ser = serial.Serial(port, baud, timeout=0.2)
        self._id = 1
        self.focus_wiring = 1

    def close(self) -> None:
        self.ser.close()

    def call(self, method: str, params: Optional[dict[str, Any]] = None) -> dict[str, Any]:
        req = {"id": self._id, "method": method, "params": params or {}}
        self._id += 1
        line = json.dumps(req, separators=(",", ":"))
        self.ser.write((line + "\n").encode("utf-8"))
        self.ser.flush()
        return self._read_response(req["id"])

    def _read_response(self, expect_id: int, deadline_s: float = 2.0) -> dict[str, Any]:
        deadline = time.monotonic() + deadline_s
        while time.monotonic() < deadline:
            raw = self.ser.readline()
            if not raw:
                continue
            text = raw.decode("utf-8", errors="replace").strip()
            if not text:
                continue
            if not text.startswith("{"):
                # Ignore non-JSON firmware chatter quietly in interactive mode
                continue
            try:
                msg = json.loads(text)
            except json.JSONDecodeError:
                # Truncated/corrupt line — do not dump the whole blob
                print("  ! skipped incomplete serial JSON")
                continue
            if msg.get("id") != expect_id:
                continue
            result = msg.get("result")
            if isinstance(result, dict) and "focusWiring" in result:
                self.focus_wiring = int(result["focusWiring"])
            return msg
        return {"id": expect_id, "error": {"code": "TIMEOUT", "message": "no response"}}

    def show(self, msg: dict[str, Any], *, full_map: bool = False) -> None:
        if "error" in msg:
            err = msg["error"]
            print(f"  ! {err.get('code', 'ERROR')}: {err.get('message', msg)}")
            return
        result = msg.get("result")
        if not isinstance(result, dict):
            print(f"  ? unexpected: {msg}")
            return
        if full_map and "assignments" in result:
            print(format_map(result))
            return
        if "focusWiring" in result:
            ok = result.get("ok")
            prefix = "ok  " if ok is True else ("fail" if ok is False else "    ")
            print(f"  {prefix} {format_focus(result)}")
            return
        print(f"  ok  {result}")


def pick_port(explicit: Optional[str]) -> str:
    if explicit:
        return explicit
    ports = list(list_ports.comports())
    teensy = [
        p
        for p in ports
        if "usbmodem" in (p.device or "").lower() or "teensy" in (p.description or "").lower()
    ]
    if len(teensy) == 1:
        return teensy[0].device
    if not ports:
        raise SystemExit("No serial ports found")
    print("Available ports:")
    for i, p in enumerate(ports):
        print(f"  [{i}] {p.device}  {p.description}")
    choice = input("Select port index: ").strip()
    return ports[int(choice)].device


def read_key() -> str:
    fd = sys.stdin.fileno()
    old = termios.tcgetattr(fd)
    try:
        tty.setraw(fd)
        ch = sys.stdin.read(1)
        if ch == "\x1b":
            return "esc"
        return ch
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old)


def main() -> int:
    parser = argparse.ArgumentParser(description="DodecaRGB interactive calibration (WASD)")
    parser.add_argument("-p", "--port", help="Serial port (default: auto-detect Teensy)")
    parser.add_argument("-b", "--baud", type=int, default=115200)
    args = parser.parse_args()

    port = pick_port(args.port)
    print(f"Opening {port} @ {args.baud}")
    print(HELP)

    client = Client(port, args.baud)
    try:
        client.show(client.call("device.getInfo"))
        client.show(client.call("calibration.begin"))
        print("Interactive mode on — watch the LEDs, use WASD.\n")

        while True:
            key = read_key()
            if key in ("q", "Q", "\x03", "esc"):
                client.call("calibration.end")
                print("bye")
                break
            if key in ("a", "A"):
                client.show(client.call("calibration.rotate", {"dir": -1}))
            elif key in ("d", "D"):
                client.show(client.call("calibration.rotate", {"dir": 1}))
            elif key in ("w", "W"):
                client.show(client.call("calibration.shift", {"dir": 1}))
            elif key in ("s", "S"):
                client.show(client.call("calibration.shift", {"dir": -1}))
            elif key in ("\r", "\n", " "):
                client.show(client.call("calibration.confirm"))
            elif key == "[":
                w = max(0, client.focus_wiring - 1)
                client.show(client.call("calibration.setFocus", {"wiringIndex": w}))
            elif key == "]":
                w = min(11, client.focus_wiring + 1)
                client.show(client.call("calibration.setFocus", {"wiringIndex": w}))
            elif key in ("r", "R"):
                client.show(client.call("calibration.reset"))
            elif key in ("g", "G"):
                client.show(client.call("calibration.get"), full_map=True)
            elif key in ("i", "I"):
                client.show(client.call("calibration.begin"))
            elif key in ("h", "H", "?"):
                print(HELP)
            else:
                print(f"(unbound key {key!r}) h for help")
    finally:
        client.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
