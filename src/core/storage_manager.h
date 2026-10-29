// MicroSD card: mounting and the folder layout.

#pragma once

/// The SD card is optional. Without it the firmware still runs; anything that
/// reads or writes files checks isReady() first.
class StorageManager {
public:
    /// Mounts the card and creates the app folders. Requires the SPI bus to be
    /// started. Returns false if the card is missing or unusable.
    bool begin();

    /// True when the card is mounted and every app folder exists.
    bool isReady() const;

private:
    bool ready_ = false;

    void logCardInfo();
    bool createFolders();
};
