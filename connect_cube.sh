#!/usr/bin/env bash
set -euo pipefail

WORKDIR="/tmp/msys"
PIDS_FILE="$WORKDIR/pids.txt"
mkdir -p "$WORKDIR"
: > "$PIDS_FILE"

declare -A GROUP_MAP=(
    [group1]="10.165.17.103 10.165.17.102 10.165.17.150 10.165.17.181"
    [group2]="10.165.17.107 10.165.17.104 10.165.17.151 10.165.17.181"
    [group3]="10.165.17.105 10.165.17.106 10.165.17.152 10.165.17.182"
    [group4]="10.165.17.109 10.165.17.108 10.165.17.153 10.165.17.182"
    [group5]="10.165.17.111 10.165.17.110 10.165.17.154 10.165.17.180"
    [group6]="10.165.17.113 10.165.17.112 10.165.17.155 10.165.17.180"
    [group7]="10.165.17.115 10.165.17.114 10.165.17.156 10.165.17.183"
    [group8]="10.165.17.117 10.165.17.116 10.165.17.157 10.165.17.184"
    [group9]="10.165.17.119 10.165.17.118 10.165.17.158 10.165.17.184"
    [group10]="10.165.17.121 10.165.17.120 10.165.17.159 10.165.17.185"
    [group11]="10.165.17.123 10.165.17.122 10.165.17.160 10.165.17.185"
    [group12]="10.165.17.125 10.165.17.124 10.165.17.161 10.165.17.186"
    [group13]="10.165.17.127 10.165.17.126 10.165.17.162 10.165.17.186"
    [group14]="10.165.17.129 10.165.17.128 10.165.17.163 10.165.17.187"
    [group15]="10.165.17.131 10.165.17.130 10.165.17.164 10.165.17.187"
)

# --- Lokale Ports ---
L_STECK=42000
L_WEBCAM=49000
L_SERVO_TARGET=40001
L_ICM_SSH=48000
L_ICM_GDB=2345
L_ICM_TARGET=40000
L_SERVO_SSH_FALLBACK=45000
L_SERVO_EXTRA=3456

# --- MSys Server ---
MSYS_USER="msys"
MSYS_HOST="129.143.166.42"
MSYS_PORT=50001

start_connection() {
    echo "[*] Starte SSH-Tunnel zu MSys-Server..."

    local group=${1:-}

    if [[ -z "$group" ]]; then
        echo "Fehler: Bitte Gruppe angeben (z. B. group5)"
        exit 1
    fi

    # Prüfen ob Gruppe existiert
    if [[ -z "${GROUP_MAP[$group]:-}" ]]; then
        echo "Ungültige Gruppe '$group'. Gültig sind: 1-15"
        exit 1
    fi

    # IPs auslesen
    read -r IP_ICMBBB IP_SERVOBBB IP_WEBCAM IP_STECKDOSE <<< "${GROUP_MAP[$group]}"

    echo "[*] Starte SSH-Tunnel für $group ..."
    echo "    ICM:        $IP_ICMBBB"
    echo "    Servo:      $IP_SERVOBBB"
    echo "    Webcam:     $IP_WEBCAM"
    echo "    Steckdose:  $IP_STECKDOSE"
    
    autossh -M 0 \
        -L $L_STECK:$IP_STECKDOSE:80 \
        -L $L_WEBCAM:$IP_WEBCAM:554 \
        -L $L_SERVO_TARGET:$IP_SERVOBBB:40000 \
        -L $L_ICM_SSH:$IP_ICMBBB:22 \
        -L $L_ICM_GDB:$IP_ICMBBB:2345 \
        -L $L_ICM_TARGET:$IP_ICMBBB:40000 \
        -L $L_SERVO_SSH_FALLBACK:$IP_SERVOBBB:22 \
        -L $L_SERVO_EXTRA:$IP_SERVOBBB:3456 \
        $MSYS_USER@$MSYS_HOST -p $MSYS_PORT

    echo "[*] SSH-Tunnel gestartet. Lokale Ports verfügbar:"
    echo "  Steckdose:   localhost:$L_STECK"
    echo "  Webcam:      localhost:$L_WEBCAM"
    echo "  Servo:       localhost:$L_SERVO_TARGET"
    echo "  ICM SSH:     localhost:$L_ICM_SSH"
    echo "  ICM GDB:     localhost:$L_ICM_GDB"
    echo "  ICM App:     localhost:$L_ICM_TARGET"
    echo "  Servo SSH:   localhost:$L_SERVO_SSH_FALLBACK"
    echo "  Servo Extra: localhost:$L_SERVO_EXTRA"
}

stop_connection() {
    echo "[*] Beende SSH-Tunnel..."
    # kill alle autossh-Prozesse zum MSys-Host
    pkill -f "autossh.*$MSYS_HOST"
    echo "[*] SSH-Tunnel beendet."
}

status_connection() {
    echo "[*] Prüfe laufende SSH-Tunnel..."
    pgrep -af "autossh.*$MSYS_HOST" || echo "Keine laufenden Tunnel gefunden."
}

action=${1:-}
group=${2:-}

case "$action" in
    start)  start_connection "$group" ;;
    stop)   stop_connection ;;
    status) status_connection ;;
    *)
        echo "Usage: $0 {start|stop|status} [group]"
        echo "Beispiel: $0 start group5"
        exit 1
        ;;
esac
