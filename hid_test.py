# Install python3 HID package https://pypi.org/project/hid/
import time

import hid


vid = 0x1D50
pid = 0x616A

try:
    h = hid.device()

    h.open(vid, pid)

    print('Manufacturer: %s' % h.get_manufacturer_string())
    print('Product: %s' % h.get_product_string())
    print('Serial No: %s' % h.get_serial_number_string())

    # enable non-blocking mode
    h.set_nonblocking(1)

    # write some data to the device
    print('Write the data')
    h.write([0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])

    # wait
    time.sleep(0.05)

    # read back the answer
    print('Read the data')
    while True:
        d = h.read(32)
        if d:
            for n in d:
                print('%02x' % n)
        else:
            break

    print('Closing the device')
    h.close()

except IOError as ex:
    print(ex)
    print("You probably don't have the hard-coded device.")
    print('Update the h.open() line in this script with the one')
    print('from the enumeration list output above and try again.')

print('Done')
