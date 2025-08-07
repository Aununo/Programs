a = 288260533169915
p = 1007621497415251

print(pow(a, (p - 1) // 2, p)) #1

def descrypt_flag(cipher):
    flag = []
    for i in cipher:
        if pow(i, (p - 1) // 2, p) == 1:
            flag.append('1')
        else:
            flag.append('0')
    flag = ''.join(flag)
    reversed(flag)
    flag = ''.join([chr(int(flag[i:i + 8], 2)) for i in range(0, len(cipher), 8)])
    return flag

def main():
    file_path = './output.txt'
    try:
        with open(file_path, 'r') as f:
            content = f.read()
        content = content.strip().replace('[', '').replace(']', '').replace(' ', '')
        cipher = [int(x) for x in content.split(',')]
        flag = descrypt_flag(cipher)
        print(f"Flag: {flag}")

    except FileNotFoundError:
        print(f"File {file_path} not found.")
        return

    

if __name__ == "__main__":
    main()


#crypto{p4tterns_1n_re5idu3s}
