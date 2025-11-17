#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORDS 5000
#define MAX_STOP 500

typedef struct {
    char word[100];
    int lines[500];
    int lineCount;
    int freq; // tổng số lần xuất hiện
} Entry;

Entry indexList[MAX_WORDS];
char stopWords[MAX_STOP][100];
int stopCount = 0;
int wordCount = 0;

/* =======================
    LOAD STOP WORDS
    ======================= */
void loadStopWords(FILE *fst) {
    char buffer[100];

    while (fgets(buffer, sizeof(buffer), fst)) {
        buffer[strcspn(buffer, "\n")] = 0;

        int i = 0, j = 0;
        char clean[100];

        while (buffer[i]) {
            if (isalpha(buffer[i])) {
                clean[j++] = tolower(buffer[i]);
            }
            i++;
        }
        clean[j] = '\0';

        if (strlen(clean) > 0) {
            if (stopCount < MAX_STOP) {
                strcpy(stopWords[stopCount++], clean);
            }
        }
    }
}

/* ======================= */
int isStopWord(char *w) {
    for (int i = 0; i < stopCount; i++)
        if (strcmp(w, stopWords[i]) == 0)
            return 1;
    return 0;
}

/* ======================= */
// prev: Ký tự phân cách đứng ngay trước từ. Dấu '.' nếu là dấu chấm câu.
int isProperNoun(const char *orig, char prev) {
    // 1. Kiểm tra chữ cái đầu có viết hoa không
    if (!isupper(orig[0])) return 0;
    
    // 2. Nếu từ viết hoa đứng sau dấu chấm câu (hoặc đầu dòng),
    // thì nó KHÔNG phải danh từ riêng (vì nó là từ đầu câu).
    if (prev == '.') return 0;
    
    // 3. Còn lại: từ viết hoa không đứng sau dấu chấm câu -> danh từ riêng (cần loại bỏ)
    return 1;
}

/* ======================= */
int findWord(char *w) {
    for (int i = 0; i < wordCount; i++)
        if (strcmp(indexList[i].word, w) == 0)
            return i;
    return -1;
}

/* ======================= */
void addLine(Entry *e, int line) {
    // Chỉ thêm số dòng nếu nó chưa được ghi nhận cho từ này ở dòng cuối cùng
    if (e->lineCount == 0 || e->lines[e->lineCount - 1] != line) {
        if (e->lineCount < 500) {
            e->lines[e->lineCount++] = line;
        }
    }
}

/* ======================= */
void addWord(char *w, int line) {
    int pos = findWord(w);

    if (pos == -1) {
        if (wordCount < MAX_WORDS) {
            strcpy(indexList[wordCount].word, w);
            indexList[wordCount].lineCount = 0;
            indexList[wordCount].freq = 1;
            addLine(&indexList[wordCount], line);
            wordCount++;
        }
    } else {
        indexList[pos].freq++;
        addLine(&indexList[pos], line);
    }
}

/* ======================= */
int cmp(const void *a, const void *b) {
    // Sắp xếp theo thứ tự từ điển
    return strcmp(((Entry *)a)->word, ((Entry *)b)->word);
}

/* ======================= */

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Usage: %s <stopw.txt> <input.txt> <output.txt>\n", argv[0]);
        return 1;
    }

    FILE *fst = fopen(argv[1], "r");
    FILE *fin = fopen(argv[2], "r");
    FILE *fout = fopen(argv[3], "w");

    if (!fst || !fin) {
        printf("Khong mo duoc file dau vao!\n");
        return 1;
    }
    if (!fout) {
        printf("Khong mo duoc file output!\n");
        if (fst) fclose(fst);
        if (fin) fclose(fin);
        return 1;
    }

    loadStopWords(fst);

    char lineBuf[2000];
    int line = 0;
    // 'prev' là ký tự phân cách ngay trước từ hiện tại
    char prev = '.'; 

    while (fgets(lineBuf, sizeof(lineBuf), fin)) {
        line++;

        // Đầu dòng luôn coi như đứng sau dấu chấm để từ đầu dòng (viết hoa) không bị coi là danh từ riêng.
        prev = '.'; 

        int len = strlen(lineBuf);
        char word[100];
        int wpos = 0;

        for (int i = 0; i <= len; i++) {
            char c = lineBuf[i];

            if (isalpha(c)) {
                word[wpos++] = c;
            } else {
                // Kết thúc một từ
                if (wpos > 0) {
                    word[wpos] = '\0';

                    char orig[100];
                    strcpy(orig, word); // Lưu từ gốc (có in hoa)

                    for (int j = 0; word[j]; j++)
                        word[j] = tolower(word[j]); // Chuyển sang chữ thường

                    if (!isStopWord(word)) {
                        // isProperNoun(orig, prev): kiểm tra từ gốc và ký tự phân cách trước đó
                        if (!isProperNoun(orig, prev)) {
                            addWord(word, line);
                        }
                    }
                    
                    // Cập nhật prev bằng ký tự phân cách vừa gặp
                    if (c == '.' || c == '?' || c == '!') {
                        prev = '.';
                    } else {
                        prev = ' '; // Đánh dấu là ký tự phân tách không phải dấu chấm câu
                    }

                    wpos = 0;
                } else {
                    // Xử lý khi gặp nhiều ký tự phân tách liên tiếp
                    if (c == '.' || c == '?' || c == '!') {
                        prev = '.';
                    } else if (!isalpha(c) && c != '\0' && c != '\n') {
                        // Mọi ký tự phân tách khác dấu chấm câu được coi là khoảng trắng.
                        if (prev != '.') prev = ' ';
                    }
                }
            }
        }
    }

    fclose(fin);
    fclose(fst);

    qsort(indexList, wordCount, sizeof(Entry), cmp);

    /* ===== OUTPUT ===== */
    fprintf(fout, "===== INDEX =====\n");

    for (int i = 0; i < wordCount; i++) {
        fprintf(fout, "%s %d", indexList[i].word, indexList[i].freq);

        for (int j = 0; j < indexList[i].lineCount; j++) {
            fprintf(fout, ", %d", indexList[i].lines[j]);
        }
        fprintf(fout, "\n");
    }

    fclose(fout);

    printf("Da ghi ket qua vao %s\n", argv[3]);

    return 0;
}