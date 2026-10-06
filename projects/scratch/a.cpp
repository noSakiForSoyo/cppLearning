void level3(int* p) { *p = 42; }
void level2(int* p) { level3(p); }
void level1(int* p) { level2(p); }

int main() {
    int* p = nullptr;
    level1(p);
}