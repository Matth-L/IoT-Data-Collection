function decodeDownlink(input) {
    let bytes = input.bytes;

    let value = (bytes[0] << 8) | bytes[1];

    let state = "normal";

    if (value == 1) {
        state = "warning";
    } else if (value == 2) {
        state = "alert";
    }
    // Gestion du signe (si valeur négative sur 16 bits)

    return {
        data: {
            securityState: state,
        }
    };
}