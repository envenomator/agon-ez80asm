counter = 0
with open('all.txt', 'r') as file:
    for line in file:
        counter = counter + 1
        filename = "test{c}.s".format(c = counter)
        with open(filename, 'x') as outputfile:
            outputfile.write(";Testing EZ80 instruction in Z80 mode\n.cpu Z80\n    ")
            outputfile.write(line)
