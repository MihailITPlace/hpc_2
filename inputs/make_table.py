with open('../results') as f:
    lines = f.readlines()

table = [[0.0] * 5 for i in range(15)]


attempt = 0
threads = 0
for l in lines:
    if l.strip() == "":
        continue
    if "==>" in l:
        parts = l.split("_")
        attempt = int(parts[1])
        threads = int(parts[-1]) - 1
        continue
    time = float(l)
    table[threads - 1][attempt - 1] = time

#print(";".join([str(i+1) + 'п' for i in range(5)]))
for i in table:
    tmp = [str(i) for i in i]
    print(";".join(tmp).replace('.', ','))
