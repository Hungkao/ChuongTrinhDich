Cách biên dịch chương trình (compile)
    gcc index.c -o index
    
Cách chạy chương trình

Cú pháp:

    ./index <stopw-file> <input-file> <output-file>
Ví dụ:

    ./index stopw.txt vanban.txt output.txt    

Giải thích:

    stopw.txt → file chứa stop words (mỗi dòng 1 từ)

    input.txt → văn bản cần lập chỉ mục

    out.txt → tệp kết quả

Format file output (out.txt)

    File kết quả có dạng:
        ===== INDEX =====
        about           94, 45, 49, 63, 75,...
        afternoon       1, 1
        gathering       2, 4, 6
        tribal          2, 4, 5
        ...

