with open('build/file_100000.txt', 'r') as f:
    txt = f.read()
    words = txt.split()

    unique_words = set(words)
    print(f"Total words: {len(words)}")
    print(f"Unique words: {len(unique_words)}")