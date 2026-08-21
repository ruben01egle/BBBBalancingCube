# BeagleBone Black — Realtime Image Setup

Realtime-capable Linux image for the BeagleBone Black, based on:
https://www.beagleboard.org/distros/beaglebone-black-debian-13-6-2026-07-24-iot-v6-18-x

Result: Debian 13 + PREEMPT_RT kernel, locked-down firewall for the MSys
tunnel, passwordless sudo for the app/debugger used by VS Code tasks.

All steps below (until "Image ready") are done over **USB SSH**, with the
board connected to the internet.

---

## 1. Connect

- User: `debian` (default). `root` available for dev if needed.
- Connect via mDNS:
  ```bash
  ssh debian@BeagleBone.local
  ```
- Fallback if mDNS fails: `ssh debian@192.168.7.2` (USB gadget IP)

---

## 2. Install the realtime kernel

Why: stock kernel isn't deterministic enough for PRU access, mmap'd
registers, GPIO/PWM timing. BeagleBoard.org's `bone-rt` kernel (not Debian's
generic `armmp`) also carries the AM335x-specific overlay/cape/PRU patches
this project needs for device tree reconfiguration.

```bash
sudo apt update
# search available modules
sudo apt search linux-image | grep bone | grep rt
sudo apt install linux-image-6.12.96-bone-rt-r64
sudo reboot
```

- Verify: `uname -r` → `6.12.96-bone-rt-r64`
- Check for a newer patch revision if this doc is stale:
  `apt-cache search linux-image | grep bone-rt`

For detailed kernel configuration, see:
```bash
zcat /proc/config.gz | grep PREEMPT
```

---

## 3. Firewall (ufw)

Why: board is fully open by default; restrict to only what dev + the MSys
tunnel need. Do this over USB — not the interface you're about to lock down.

| Port  | Proto | Purpose |
|-------|-------|---------|
| 22    | tcp   | SSH |
| 2345  | tcp   | GDB remote |
| 40000 | tcp   | Application target port |

```bash
sudo ufw allow 22/tcp
sudo ufw allow 2345/tcp
sudo ufw allow 40000/tcp
sudo ufw enable
sudo ufw status verbose
```

- No static IP/routes needed — MSys routes to each board by MAC address,
  plain DHCP on `eth0` is enough.
- Double-check port/protocol when copying rules between images (e.g.
  `4000` vs `40000`, `udp` vs `tcp`) — a typo breaks the tunnel silently,
  with no firewall error.

---

## 4. Update & install packages

```bash
sudo apt update && sudo apt full-upgrade -y
sudo apt install <project packages, e.g. python3-pip, gdb, build-essential, ...>
```

Do this before cloning the SD card so every board starts from the same
baseline.

---

## 5. Passwordless sudo for app + debugger

Why: VS Code tasks run the app and `gdb` via `sudo` (needed for raw hardware
access). Prompting for a password on every task run is impractical, so add
a narrowly scoped `NOPASSWD` rule — only for the fixed app path and
debugger, not `NOPASSWD: ALL`.

```bash
sudo visudo -f /etc/sudoers.d/noPw
```

Add (adjust paths to match `Makefile` / `manage_cube.sh`):

```
debian ALL=(ALL) NOPASSWD: /path/to/app
debian ALL=(ALL) NOPASSWD: /usr/bin/gdbserver
```

- Use `/etc/sudoers.d/` (not editing `/etc/sudoers` directly) → isolated,
  easy to remove.
- `visudo` validates syntax before saving → typo can't lock out sudo.

Set rights with:
```bash
sudo chmod 0440 /etc/sudoers.d/noPW
```
Sudo will not require a pw anymore for those cmds. Use sudo -n to ensure no pw quest.

---

## 6. Image ready

Clone/flash the SD card to the remaining boards. Each will boot with:

- the realtime kernel
- only SSH / GDB / app port exposed
- MSys-tunnel reachability, no per-board network config
- passwordless sudo for app + debugger via VS Code tasks
