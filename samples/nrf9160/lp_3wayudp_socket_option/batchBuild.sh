#### batch build 

west build -b nrf9160dk_nrf9160_ns -p -- -DOVERLAY_CONFIG=nb_b28.conf
mv build/zephyr/merged.hex ./nb_b28.hex

west build -b nrf9160dk_nrf9160_ns -p -- -DOVERLAY_CONFIG=nb_b3_8_28.conf
mv build/zephyr/merged.hex ./nb_b3_8_28.hex

west build -b nrf9160dk_nrf9160_ns -p -- -DOVERLAY_CONFIG=nb_b28_dbg_trace.conf
mv build/zephyr/merged.hex ./nb_b28_dbg_trace.hex

west build -b nrf9160dk_nrf9160_ns -p -- -DOVERLAY_CONFIG=nb_b3_8_28_dbg_trace.conf
mv build/zephyr/merged.hex ./nb_b3_8_28_dbg_trace.hex

west build -b nrf9160dk_nrf9160_ns -p -- -DOVERLAY_CONFIG=m1_b3_8_28.conf
mv build/zephyr/merged.hex ./m1_b3_8_28.hex

west build -b nrf9160dk_nrf9160_ns -p -- -DOVERLAY_CONFIG=m1_b3_8_28_dbg_trace.conf
mv build/zephyr/merged.hex ./m1_b3_8_28_dbg_trace.hex