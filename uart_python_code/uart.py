import serial
import time
# Configuration
serial_port = "/dev/ttyUSB1"
baud_rate = 1600000
memory = []
count=0
# Open serial port
with serial.Serial(serial_port, baud_rate) as device:
    print("Starting data collection...")


    while True:
        # start = time.time()
       #
       # 
       # input("Press any key :")

        

        data_bytes = device.read(1)  # Read one byte


        if not data_bytes:
            print("No data received, waiting...")
            time.sleep(0.1)  # Sleep briefly to allow data to accumulate
            continue
        if len(data_bytes) == 1:
            byte1 = data_bytes
            print("byte =", byte1) 
            
           # input("Press any key :")
        if byte1 != b'z':
            count+=1
            memory.append(int.from_bytes(byte1,'big'))
            print("count=",count)
            print("memory length",len(memory))
            if count>=256:
                print("Break")
                break
                
        else:
            print("memory length",len(memory))
            #    if len(memory) != 0:
                    # print("Angle =", self.memory[0])
                    # print("2 =", self.memory[1])
                    # print("3 =", self.memory[2])
                    # print("4 =", self.memory[3])
                    #bytes_dist = bytes([memory[0], memory[1]])
                    #int_bytes_dist = int.from_bytes(bytes_dist, byteorder='big')
            #        print(count)
                    #print("byte value =", memory[count-1])
                    #voltage = (int_bytes_dist / 4096) * 3.3  # distance in cm
                    #print("Voltage =", voltage)
            #        print("byte value =", memory[count-1])

              
                   # memory.clear()
                  

# Data processing




processed_data = []
i = 0
while i < len(memory) - 3:  # Make sure we have at least 4 bytes to process
    if len(memory[i:i+4]) == 4:
        # Combine distance bytes and convert to integer
        distance = (memory[i] + memory[i+1]*256)
        sigma =(memory[i+2])
        status = (memory[i+3])
        processed_data.append((distance, sigma, status))
        i += 4  # Move to the next set of data

# Write to file
with open('received_data.txt', 'w') as file:
    # Preparing to group the data in chunks of 8
    distances = []
    sigmas = []
    statuses = []

    for idx, (distance, sigma, status) in enumerate(processed_data):
        # Collecting each attribute
        distances.append(f"distance = {distance} mm")
        sigmas.append(f"sigma = {sigma} mm")
        statuses.append(f"status = {status}")

        # Every 8 measurements, we write the grouped attributes to the file
        if (idx + 1) % 8 == 0:
            file.write(" | ".join(distances) + "\n")
            file.write(" | ".join(sigmas) + "\n")
            file.write(" | ".join(statuses) + "\n")
            file.write("---------------------------------------------------------------------------------\n")


            print(" | ".join(distances))
            print(" | ".join(sigmas))
            print(" | ".join(statuses))
            print("---------------------------------------------------------------------------------")
            # Clear lists for the next set of 8
            distances = []
            sigmas = []
            statuses = []

    # If the total number of entries is not a multiple of 8, write remaining data
    if distances:
        file.write(" | ".join(distances) + "\n")
        file.write(" | ".join(sigmas) + "\n")
        file.write(" | ".join(statuses) + "\n")
        file.write("---------------------------------------------------------------------------------\n")


        print(" | ".join(distances))
        print(" | ".join(sigmas))
        print(" | ".join(statuses))
        print("---------------------------------------------------------------------------------")

