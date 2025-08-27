def parse_file():
    file = open("map.crm", "r")

    list = []

    for line in file.readlines():
        point1 = line.split(";")[0].split(":")
        point2 = line.split(";")[1].split(":")
        list.append([int(point1[0]), int(point1[1]), int(point2[0]), int(point2[1])])

    return list


parse_file()
