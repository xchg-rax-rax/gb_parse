#!/bin/python3

def main():
    # open file 
    with open('../docs/old_licensee_codes.txt', 'r') as fd:
        lines = fd.readlines()
        for line in lines:
            tokens = line.split()
            value = tokens[0]
            name = '_'.join([token.upper() for token in tokens[1:]])
            print('#define OLD_LICENSEE_'+name+' 0x'+value)

    with open('../docs/old_licensee_codes.txt', 'r') as fd:
            lines = fd.readlines()
            for line in lines:
                tokens = line.split()
                name = '_'.join([token.upper() for token in tokens[1:]])
                description = ' '.join([token.title() for token in tokens[1:]])
                print('case OLD_LICENSEE_'+name+':')
                print(f'    printf("{description}");')
                print('    break;')

if __name__ == "__main__":
    main()
