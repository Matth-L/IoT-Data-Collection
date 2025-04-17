function decodeDownlink(input) {
    let bytes = input.bytes;

    let value = (bytes[0] << 8) | bytes[1];

    let warning = false;
    let alarm = false;

    if (value == 1) {
        warning = true;
    } else if (value == 2) {
        alarm = true;
    }
    // Gestion du signe (si valeur négative sur 16 bits)

    return {
        data: {
            value: value,
            warning: warning,
            alarm: alarm
        }
    };
}