def test():
    print("test")

def test1():
    print("test1")
    test()

def main():
    print("main")
    test()
    test1()

if __name__ == "__main__":
    main()
