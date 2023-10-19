filename = "uploaddone"
file = open(filename + ".html", "r") 
data = file.readlines();
file.close()

file = open(filename, "w")
file.write(" data = {")
i = 0
for line in data:
    for item in line:
        file.write(hex(ord(item)) + ", ")
        i = i + 1
        if(i == 6):
            file.write("\n        ")
            i = 0
        # print(line)
file.write(" }")
file.close()