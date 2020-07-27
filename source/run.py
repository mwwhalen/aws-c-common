
import sys;
import os;

def run_commands(fbase):
    os.system('rm *.bc')
    os.system('rm *.ll')
    os.system('rm cex.exe')
    # str = 'sea fe -m64 -I../include -o ' + fbase + '.bc -g ' + fbase + '.c';
    # print(str);
    # os.system(str);
    # str = 'llvm-dis ' + fbase + '.bc';
    # print(str);
    # os.system(str)
    # print('\n\n\n!!!About to solve!!!\n\n\n')
    str = 'sea pf -m64 --inline -I../include --show-invars -o ' + fbase + '.smt2 ' + fbase + '.c ' + \
            '--cex=cex.ll --log=cex --bv-cex --verbose=1 --horn-strictly-la=false --horn-use-write=true';
    print(str);
    os.system(str);
    str = 'sea exe -I../include -m64 ' + fbase + '.c cex.ll -o cex.exe'
    print(str);
    os.system(str);


if __name__ == "__main__":
    print("Arguments count: " + str(len(sys.argv)))
    for i, arg in enumerate(sys.argv):
        print("Argument: " + arg)
    run_commands(sys.argv[1])

