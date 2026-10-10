"""Live Flipper One display, using the repository's existing RPC client."""
from pathlib import Path
import os
import queue
import threading
import tkinter as tk
from tkinter import messagebox
from PIL import Image, ImageTk
from serial.tools import list_ports
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "rpc_test"))
import rpc_test as rpc

HERE = Path(__file__).resolve().parent





def main():
    root = tk.Tk()
    root.title('Flipper One — USB Display')
    root.configure(bg='#202024')
    root.geometry('814x590+100+80')
    root.resizable(False, False)
    tk.Label(root, text='FLIPPER ONE  ·  Live USB Display', bg='#202024', fg='#ff963d',
             font=('Segoe UI', 16, 'bold')).pack(pady=(12, 6))
    screen = tk.Label(root, bg='white', text='Connecting…')
    screen.pack(padx=20, pady=4)
    status = tk.StringVar(value='Looking for device…')
    tk.Label(root, textvariable=status, bg='#202024', fg='#dddddd').pack(pady=4)
    controls = tk.Frame(root, bg='#202024')
    controls.pack(pady=4)
    tk.Label(root, text='Arrows: navigate  ·  Enter: OK  ·  Esc: back  ·  4: settings  ·  Tab: tests',
             bg='#202024', fg='#bbbbbb').pack()
    tk.Label(root, text='USB development build · Peripheral hardware and persistent settings unavailable',
             bg='#202024', fg='#999999').pack(pady=3)
    stop = threading.Event()
    frames = queue.Queue(maxsize=3)
    pressed = set()
    port = None

    try:
        ports = [p for p in list_ports.comports() if (p.vid, p.pid) == (0x37c1, 0xf101)]
        if len(ports) != 1:
            raise RuntimeError('Expected exactly one Flipper USB device. Connect one device and reopen the viewer.')
        rpc._ensure_protos()
        port = rpc.open_serial(ports[0].device)
        if not rpc.enter_rpc_mode(port):
            raise RuntimeError('The device did not enter RPC mode. Check that the GUI firmware is installed.')
        rpc.send_start_virtual_display(port)
    except Exception as exc:
        if port: port.close()
        messagebox.showerror('Connection error', str(exc) + '\nClose other serial-port applications such as PuTTY.', parent=root)
        root.destroy()
        return

    def send(button, action):
        try:
            rpc.send_button(port, button, action)
        except Exception as exc:
            status.set('Connection error: ' + str(exc))

    def click(button):
        send(button, 'PRESS')
        send(button, 'RELEASE')

    for caption, button in [('Help', 'KEY_2'), ('Power', 'POWER'), ('Settings', 'KEY_4'),
                             ('↑', 'UP'), ('↓', 'DOWN'), ('←', 'LEFT'), ('→', 'RIGHT'),
                             ('OK', 'OK'), ('Back', 'BACK'), ('Tests', 'SW')]:
        tk.Button(controls, text=caption, command=lambda b=button: click(b),
                  takefocus=False).pack(side='left', padx=2)
    keys = {'Up':'UP','Down':'DOWN','Left':'LEFT','Right':'RIGHT','Return':'OK',
            'space':'OK','Escape':'BACK','BackSpace':'BACK','Tab':'SW','1':'KEY_1',
            '2':'KEY_2','3':'POWER','4':'KEY_4','5':'KEY_5'}
    def key_down(event):
        button = keys.get(event.keysym)
        if button and button not in pressed:
            pressed.add(button)
            send(button, 'PRESS')
        if button: return 'break'
    def key_up(event):
        button = keys.get(event.keysym)
        if button in pressed:
            pressed.remove(button)
            send(button, 'RELEASE')
        if button: return 'break'
    root.bind('<KeyPress>', key_down)
    root.bind('<KeyRelease>', key_up)

    def reader():
        while not stop.is_set():
            try:
                msg = rpc.read_message(port)
                if msg and msg['which'] == 'frame':
                    if frames.full(): frames.get_nowait()
                    frames.put_nowait(msg)
            except Exception:
                if not stop.is_set(): stop.set()
                break
    worker = threading.Thread(target=reader, daemon=True)
    worker.start()
    count = 0
    def refresh():
        nonlocal count
        try:
            while True:
                frame = frames.get_nowait()
                assert frame['encoding'] == 'PLAIN' and frame['pixel_format'] == 'L8'
                image = Image.frombytes('L', (frame['width'], frame['height']), frame['data'])
                screen.image = ImageTk.PhotoImage(image.resize((774,432), Image.Resampling.NEAREST))
                screen.configure(image=screen.image, text='')
                count += 1
                if count == 1:
                    print('Live framebuffer displayed: 258x144', flush=True)
                status.set(f"{ports[0].device} · {frame['width']}×{frame['height']} · Frames received: {count}")
        except queue.Empty:
            pass
        if stop.is_set(): status.set('Disconnected. Close and reopen the viewer.')
        root.after(40, refresh)

    def close():
        for button in list(pressed): send(button, 'RELEASE')
        try:
            rpc.send_stop_virtual_display(port)
            rpc.send_rpc_session_close(port)
        except Exception:
            pass
        stop.set()
        worker.join(timeout=1.2)
        port.close()
        root.destroy()
    root.protocol('WM_DELETE_WINDOW', close)
    root.after(40, refresh)
    root.focus_force()
    root.mainloop()

if __name__ == '__main__':
    main()
