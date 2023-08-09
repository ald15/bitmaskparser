#include <iostream>
#include <cstdlib>
#include <list>
#include <string.h>

using namespace std;

struct bitMask {
    short* mask;
    char* expression, *debugMask;
    long long maskLen,
              mode,
              startPos;
    bool debugMode;
    list <char> st;
    bitMask(): mask(nullptr), expression(nullptr), debugMask(nullptr),
               maskLen(0), mode(0), startPos(0), debugMode(false)
    {
        if (getExpression()) parseExpression();
    }
    ~bitMask() {delete [] mask;}  
    bool getExpression();
    void parseExpression();
    friend ostream& operator << (ostream &out, const bitMask &bit_mask);
};

bool checkBrackets(const char* expression) {
    list <char> st;
    long long i = -1;
    while (expression[++i] != '\0') {
        if (expression[i] == '{') st.push_back(expression[i]);
        if (expression[i] == '}') {
            if (st.back() == '{') st.pop_back();
            else return 0;
        }
    }
    return st.empty();
}

bool checkExpression(const char* expression) {
    long long i = -1;
    bool found = false;
    char valid[] = {'{', '}', '-', ',', '_'};
    while (expression[++i] != '\0') {
        found = false;
        for (long long j = 0; j < (long long) (sizeof(valid)/sizeof(char)); j++)
            if ((expression[i] == valid[j]) || ('0' <= expression[i] && expression[i] <= '9')) {found = true; break;}
        if (!found) return false;
    }
    return true;
}

bool bitMask::getExpression() {
    long long i = 0, j = 1, k = -1;
        expression = getenv("SYMCC_MASK");
        debugMask = getenv("SYMCC_MASK_DEBUG");
        if (debugMask) {
            int debugMaskLen = -1;
            while (debugMask[k++] != '\0') debugMaskLen++;
            if (debugMaskLen == 1) {
                if (debugMask[0] == '1') debugMode = true;
                else debugMode = false;
            } else if (2 <= debugMaskLen && debugMaskLen <= 5) {
                if (!strcmp(debugMask, "yes")) debugMode = true;
                else if (!strcmp(debugMask, "no")) debugMode = false;
                else if (!strcmp(debugMask, "true")) debugMode = true;
                else if (!strcmp(debugMask, "false")) debugMode = false;
            }
            else {
                cout << "[DEBUG]{INFO} debugging mode is DISABLED. Have you tried to enable debugging mode? \n \
                If yes then the correct values of the environment variable SYMCC_DEBUG_MASK: 1, 0, true, false, yes, no." << endl;
            }
        }
        if(expression) {
            if (debugMode) cout << "[DEBUG]{INFO} Your mask expression: " << expression << '\n';
            while (expression[i] != ':')
                if ('0' <= expression[i] && expression[i] <= '9') st.push_back(expression[i++]);
                else {
                    cout << "[DEBUG]{ERROR} Error in specifying the length of the mask (example,\n \
                    the string should start with number like 1234 and then ':' => 1234:)!" << endl;
                    exit(-1);
                }   
            if (st.empty()) {
                cout << "[DEBUG]{ERROR} Error you did not specify the length of the mask before the sign ':'!" << endl;
                exit(-1);
            }
            while (!st.empty()) {
                maskLen += (st.back() - '0')*j;
                st.pop_back();
                j *= 10;
            }
            i++; j = 1;
            while (expression[i] != ':')
                if ('0' <= expression[i] && expression[i] <= '9') st.push_back(expression[i++]);
                else {
                    cout << "[DEBUG]{ERROR} Error in specifying the mode of the mask parse (there are 2 modes available for operation: 0, 1\n \
                    ,for more information, see the instructions)!" << endl;
                    exit(-1);
                } 
            if (st.empty()) {
                cout << "[DEBUG]{ERROR} Error you did not specify the mode of mask parse before the sign ':' (example, 123:0:)!" << endl;
                exit(-1);
            } 
            while (!st.empty()) {
                mode += (st.back() - '0')*j;
                st.pop_back();
                j *= 10;
            }
            if (mode == 0) mode = 0;
            else if (mode == 1) mode = 1;
            else {
                cout << "[DEBUG]{ERROR} You have entered an invalid parser mode (example, 123:0:...,there are 2 modes available for operation: 0, 1\n \
                    ,for more information, see the instructions)!" << endl;
                exit(-1);
            }
            startPos = i;
            mask = new short[maskLen];
            for (long long i = 0; i < maskLen; i++) mask[i] = 0;
            
            if (debugMode) cout << "[DEBUG]{INFO}\tExpression length is " << maskLen << " letters" << endl;
            return true;
        } else {
            if (debugMode) cout << "[DEBUG]{INFO} Compile without bit-mask" << endl;
            return false;
        }
}

void bitMask::parseExpression() {
    long long pos = 0, i = startPos, j = 1, curLen = 0, amount = 0, start = 0, temp = 0;
    bool fRepeat = false, cRepeat = false, range = false, empty = true;
    if (!checkBrackets(expression)) {
        cout << "[DEBUG]{ERROR} the specified mask contains an invalid bracket sequence!" << endl;
        exit(-1);
    }
    if (!checkExpression(&expression[startPos+1])) {
        cout << "[DEBUG]{ERROR} The input string of the mask contains invalid characters. See the description of how to make the right mask!" << endl;
        exit(-1);
    }
    if (mode == 0)
        while (expression[i++] != '\0') {
            if (pos > maskLen) {
                cout << "[DEBUG]{ERROR} The size of the specified mask (" << maskLen << ") does not match the length of the specified expression!" << endl;
                exit(-1);
            }
            if (expression[i] == '{' || expression[i] == '\0') {
                if (cRepeat) {
                    cRepeat = false;
                    while (!st.empty()) {
                        amount += st.back()*j;
                        st.pop_back();
                        j *= 10;
                    }
                    j = 1;
                    amount--;
                    for (long long l=pos; l < pos+curLen*amount; l++) mask[l] = mask[l-curLen];
                    pos += curLen * amount;
                    amount = curLen = 0;
                }

            } else if ('0' <= expression[i] && expression[i] <= '9')
                if (!cRepeat) {
                    if (expression[i] == '0' || expression[i] == '1') {
                        mask[pos++] = expression[i] - '0';
                        curLen++;
                    }
                    else {
                        cout << "[DEBUG]{ERROR} You use characters other than 0 and 1 to compose a binary mask, check if the expression is entered correctly!" << endl;
                        exit(-1);
                    }
                } else st.push_back(expression[i] - '0');
            else if (expression[i] == '}') fRepeat = true;
            else if(expression[i] == '_') {
                if (fRepeat) {
                    cRepeat = true; fRepeat = false;
                } else {
                    cout << "[DEBUG]{ERROR} Error in using the sign '_', you did not specify which sequence to repeat '{...}', example: {1001}_5!" << endl;
                    exit(-1);
                }
            }
        }
    else if (mode == 1) {
        while (expression[i++] != '\0') {
            if ((expression[i] == ',' && i+1<maskLen && expression[i+1]!='\0') || expression[i] == '\0') {
                if (st.empty()) {
                    cout << "[DEBUG]{ERROR} You have entered an invalid index, correct the error and try again!" << endl;
                    exit(-1);
                }
                while (!st.empty()) {
                    temp += st.back()*j;
                    st.pop_back();
                    j *= 10;
                }
                if (range) {
                    range = false;
                    if ((start <= temp) && (0 <= start) && (temp < maskLen)) {
                        for (long long l = start; l <= temp; l++) mask[l] = 1;
                        start = temp = 0;
                    } else {
                        cout << "[DEBUG]{ERROR} You have entered an invalid index, correct the error and try again!" << endl;
                        exit(-1);
                    }
                } else {
                    if (empty) break;
                    else if (0 <= temp && temp < maskLen) {mask[temp] = 1; temp = 0;}
                    else {
                        cout << "[DEBUG]{ERROR} You have entered an invalid index, correct the error and try again!" << endl;
                        exit(-1);
                    }
                }
                j = 1;
            } else if (expression[i] == '-') {
                if (!empty && !range) {
                    range = true;
                    while (!st.empty()) {
                        temp += st.back()*j;
                        st.pop_back();
                        j *= 10;
                    }
                    start = temp;
                    temp = 0;
                    j = 1;
                } else {
                    cout << "[DEBUG]{ERROR} You have entered an invalid index, correct the error and try again!" << endl;
                    exit(-1);
                }
            } else if ('0' <= expression[i] && expression[i] <= '9') {
                if (empty) empty = false;
                st.push_back(expression[i] - '0');
            }
            else {
                cout << "[DEBUG]{ERROR} Invalid spelling of an expression in parse mode 1, see the instructions!" << endl;
                exit(-1);
            }
        }
    } else {
         cout << "[DEBUG]{ERROR} An invalid parser operation mode is specified, look at the instructions, two modes are currently implemented: 0 and 1!" << endl;
         exit(-1);
    }
}

ostream& operator << (ostream &out, const bitMask &bit_mask) {
     if (bit_mask.debugMode) {
        out << "[DEBUG]{INFO} Current mask:\t";
            for (long long i = 0; i<bit_mask.maskLen; i++) {
                cout << bit_mask.mask[i];
            }
        cout << endl;
    }
    return out;
}

int main() {
    // Usage example
    bitMask a;
    cout << a;
	return 0;
}