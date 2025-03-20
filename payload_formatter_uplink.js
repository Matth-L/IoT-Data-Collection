function decodeUplink(input) {
    let bytes = input.bytes;

    let temperature = (bytes[0] << 8) | bytes[1];
    let humidity = (bytes[2] << 8) | bytes[3];
    let sensor = bytes[4];

    // Gestion du signe (si valeur négative sur 16 bits)
    if (temperature & 0x8000) {
        temperature = temperature - 0x10000;
    }

    return {
        data: {
            temperature: temperature,
            humidity : humidity,
            sensor: sensor
        }
    };
}