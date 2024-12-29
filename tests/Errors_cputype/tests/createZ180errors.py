counter = 0
with open('z180-ez80_extra.txt', 'r') as file:
    for line in file:
        counter = counter + 1
        filename = "z180errortest{c}.s".format(c = counter)
        with open(filename, 'x') as outputfile:
            outputfile.write(";Testing EZ80 instruction in Z180 mode\n.cpu Z180\n    ")
            outputfile.write(line)
