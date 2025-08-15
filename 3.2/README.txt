<!-- Use commands alternatives given below if you are facing any issues in executing -->

Instead of echo <pid> > /sys/kernel/mem_stats/pid
use        echo <pid> | sudo tee /sys/kernel/mem_stats/pid

Instead of echo <unit> > /sys/kernel/mem_stats/unit
use        echo <unit> | sudo tee /sys/kernel/mem_stats/unit
