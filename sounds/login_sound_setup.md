# Login Sound Setup on Rocky Linux

This document summarizes the steps taken to configure a login sound for the `modulo` user on a Rocky Linux system.

## Goal
To play a specific sound file (`startup.ogg`) automatically upon graphical login for the user `modulo`.

## Steps Performed:

1.  **Sound File Chosen:** `startup.ogg` (from the original directory `/home/moduloserver/modulo/share/audio/os_sounds/`)

2.  **Local Sound Directory Creation:**
    A local directory was created within the `modulo` user's home directory to store the sound file.
    ```bash
    mkdir -p /home/modulo/.local/share/sounds/
    ```

3.  **Sound File Copy:**
    The `startup.ogg` file was copied from its original location (which was on a CIFS mounted network drive) to the new local directory.
    ```bash
    cp /home/moduloserver/modulo/share/audio/os_sounds/startup.ogg /home/modulo/.local/share/sounds/startup.ogg
    ```

4.  **Autostart Directory Creation:**
    The necessary autostart directory for the `modulo` user was ensured to exist.
    ```bash
    mkdir -p /home/modulo/.config/autostart/
    ```

5.  **Startup Entry File Creation:**
    A `.desktop` file was created in the `modulo` user's autostart directory. This file instructs the desktop environment to execute the sound playing command on login.
    The file `login-sound.desktop` was created at `/home/modulo/.config/autostart/login-sound.desktop` with the following content:

    ```ini
    [Desktop Entry]
    Type=Application
    Name=Login Sound
    Comment=Play a sound at login
    Exec=paplay /home/modulo/.local/share/sounds/startup.ogg
    Terminal=false
    Hidden=false
    NoDisplay=true
    X-GNOME-Autostart-enabled=true
    ```

## Result:
Upon the next graphical login of the `modulo` user, the `startup.ogg` sound will play from the local machine.

## Important Note:
The command `paplay` is used to play the sound. This command is typically provided by the `pulseaudio-utils` package. If the sound does not play, you may need to install this package:
```bash
sudo dnf install pulseaudio-utils
```
