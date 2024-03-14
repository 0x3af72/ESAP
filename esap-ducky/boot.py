import storage

production = True

if production:
    storage.disable_usb_drive()