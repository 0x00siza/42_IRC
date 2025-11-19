#!/bin/bash
# Connect 5 clients quickly then disconnect them
for i in {1..5}; do
    (echo -e "PASS mew\r\nNICK user$i\r\nUSER user$i 0 * :User $i\r\nQUIT\r\n" | nc localhost 6667) &
done
wait