import subprocess

CMD = "od -vAn -N2560 -tu1 < /dev/qrandom0"
# CMD = 'echo "1 2 1 3 4 5 1 6\n 7 0 0 0 0 0 0 \n0 0 0\n 0 0 0 2 1 6 "' for testing purposes

result = subprocess.run(['sudo', 'bash', '-c', CMD], stdout=subprocess.PIPE).stdout.decode('utf-8')

numbers = [int(x) for line in result.split('\n') for x in line.split(' ') if x]

print("numbers received: ", numbers)
CNT = 10

for i in range(0, len(numbers)-CNT):
    x = numbers[i]
    different = False
    for j in range(i+1, i+CNT):
        if numbers[j] != x:
            different = True
    if different == False:
        print("TEST FAILED:",f"there are at least {CNT} {x}s in a row")
        exit()

print("Everything seems to be fine.")