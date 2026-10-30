// MicroSD card: mounting and the folder layout.

#pragma once

/// The SD card is optional. Without it the firmware still runs; anything that
/// reads or writes files checks isReady() first.
class StorageManager {
public:
    /// Mounts the card and creates the app folders. Requires the SPI bus to be
    /// started. Can be called again to re-check the card, e.g. after one is
    /// inserted. Returns false if the card is missing or unusable.
    bool begin();

    /// True when the card is mounted and every app folder exists.
    bool isReady() const;

    /// Writes a test file, reads it back and deletes it. Returns true if the
    /// data read back is identical to the data written.
    bool testReadWrite();

private:
    bool ready_ = false;

    void logCardInfo();
    bool createFolders();
};
