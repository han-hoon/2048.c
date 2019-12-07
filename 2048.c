#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>

int SIZE;
bool set_time = true;
bool goal, goal1, goal2;
uint32_t score=0;
uint8_t scheme=0;

void getColor(uint8_t value, char *color, size_t length) {
    uint8_t original[] = {8,255,1,255,2,255,3,255,4,255,5,255,6,255,7,255,9,0,10,0,11,0,12,0,13,0,14,0,255,0,255,0};
    uint8_t blackwhite[] = {232,255,234,255,236,255,238,255,240,255,242,255,244,255,246,0,248,0,249,0,250,0,251,0,252,0,253,0,254,0,255,0};
    uint8_t bluered[] = {235,255,63,255,57,255,93,255,129,255,165,255,201,255,200,255,199,255,198,255,197,255,196,255,196,255,196,255,196,255,196,255};
    uint8_t *schemes[] = {original,blackwhite,bluered};
    uint8_t *background = schemes[scheme]+0;
    uint8_t *foreground = schemes[scheme]+1;
    if (value > 0) while (value--) {
        if (background+2<schemes[scheme]+sizeof(original)) {
            background+=2;
            foreground+=2;
        }
    }
    snprintf(color,length,"\033[38;5;%d;48;5;%dm",*foreground,*background);
}

void drawBoard(uint8_t board[SIZE][SIZE]) {
    uint8_t x,y;
    char color[40], reset[] = "\033[m";
    printf("\033[H");
    
    printf("1P %21d pts\n\n",score);
    
    for (y=0;y<SIZE;y++) {
        for (x=0;x<SIZE;x++) {
            getColor(board[x][y],color,40);
            printf("%s",color);
            printf("       ");
            printf("%s",reset);
        }
        printf("\n");
        for (x=0;x<SIZE;x++) {
            getColor(board[x][y],color,40);
            printf("%s",color);
            if (board[x][y]!=0) {
                char s[8];
                snprintf(s,8,"%u",(uint32_t)1<<board[x][y]);
                uint8_t t = 7-strlen(s);
                printf("%*s%s%*s",t-t/2,"",s,t/2,"");
            } else {
                printf("   ·   ");
            }
            printf("%s",reset);
        }
        printf("\n");
        for (x=0;x<SIZE;x++) {
            getColor(board[x][y],color,40);
            printf("%s",color);
            printf("       ");
            printf("%s",reset);
        }
        printf("\n");
    }
    printf("\n");
    printf("        a,w,d,s or q        \n");
    printf("\033[A"); // one line up
}

void drawBoard2(uint8_t board[SIZE][SIZE]) {
    uint8_t x,y;
    char color[40], reset[] = "\033[m";
    printf("\33[H");
    
    printf("\t\t\t\t\t\t\t2P %21d pts\n\n",score);
    
    for (y=0;y<SIZE;y++) {
        printf("\t\t\t\t\t\t\t");
        for (x=0;x<SIZE;x++) {
            getColor(board[x][y],color,40);
            printf("%s",color);
            printf("       ");
            printf("%s",reset);
        }
        printf("\n");
        printf("\t\t\t\t\t\t\t");
        for (x=0;x<SIZE;x++) {
            
            getColor(board[x][y],color,40);
            printf("%s",color);
            if (board[x][y]!=0) {
                char s[8];
                snprintf(s,8,"%u",(uint32_t)1<<board[x][y]);
                uint8_t t = 7-strlen(s);
                printf("%*s%s%*s",t-t/2,"",s,t/2,"");
            } else {
                printf("   ·   ");
            }
            printf("%s",reset);
            
        }
        printf("\n");
        printf("\t\t\t\t\t\t\t");
        for (x=0;x<SIZE;x++) {
            getColor(board[x][y],color,40);
            printf("%s",color);
            printf("       ");
            printf("%s",reset);
        }
        printf("\n");
        
    }
    printf("\n");
    printf("\t\t\t\t\t\t\t");
    printf("        ←,↑,→,↓ or q        \n");
    printf("\033[A"); // one line up
}


uint8_t findTarget(uint8_t array[SIZE],uint8_t x,uint8_t stop) {
    uint8_t t;
    // if the position is already on the first, don't evaluate
    if (x==0) {
        return x;
    }
    for(t=x-1;;t--) {
        if (array[t]!=0) {
            if (array[t]!=array[x]) {
                // merge is not possible, take next position
                return t+1;
            }
            return t;
        } else {
            // we should not slide further, return this one
            if (t==stop) {
                return t;
            }
        }
    }
    // we did not find a
    return x;
}

bool slideArray(uint8_t array[SIZE]) {
    bool success = false;
    uint8_t x,t,stop=0;
    for (x=0;x<SIZE;x++) {
        if (array[x]!=0) {
            t = findTarget(array,x,stop);
            // if target is not original position, then move or merge
            if (t!=x) {
                // if target is zero, this is a move
                if (array[t]==0) {
                    array[t]=array[x];
                } else if (array[t]==array[x]) {
                    // merge (increase power of two)
                    array[t]++;
                    // increase score
                    score+=(uint32_t)1<<array[t];
                    if(score >= 4000) {
                        goal = true;
                    }
                    // set stop to avoid double merge
                    stop = t+1;
                }
                array[x]=0;
                success = true;
            }
        }
    }
    return success;
}

void rotateBoard(uint8_t board[SIZE][SIZE]) {
    uint8_t i,j,n=SIZE;
    uint8_t tmp;
    for (i=0; i<n/2; i++) {
        for (j=i; j<n-i-1; j++) {
            tmp = board[i][j];
            board[i][j] = board[j][n-i-1];
            board[j][n-i-1] = board[n-i-1][n-j-1];
            board[n-i-1][n-j-1] = board[n-j-1][i];
            board[n-j-1][i] = tmp;
        }
    }
}

bool moveUp(uint8_t board[SIZE][SIZE]) {
    bool success = false;
    uint8_t x;
    for (x=0;x<SIZE;x++) {
        success |= slideArray(board[x]);
    }
    return success;
}

bool moveLeft(uint8_t board[SIZE][SIZE]) {
    bool success;
    rotateBoard(board);
    success = moveUp(board);
    rotateBoard(board);
    rotateBoard(board);
    rotateBoard(board);
    return success;
}

bool moveDown(uint8_t board[SIZE][SIZE]) {
    bool success;
    rotateBoard(board);
    rotateBoard(board);
    success = moveUp(board);
    rotateBoard(board);
    rotateBoard(board);
    return success;
}

bool moveRight(uint8_t board[SIZE][SIZE]) {
    bool success;
    rotateBoard(board);
    rotateBoard(board);
    rotateBoard(board);
    success = moveUp(board);
    rotateBoard(board);
    return success;
}

bool findPairDown(uint8_t board[SIZE][SIZE]) {
    bool success = false;
    uint8_t x,y;
    for (x=0;x<SIZE;x++) {
        for (y=0;y<SIZE-1;y++) {
            if (board[x][y]==board[x][y+1]) return true;
        }
    }
    return success;
}

uint8_t countEmpty(uint8_t board[SIZE][SIZE]) {
    uint8_t x,y;
    uint8_t count=0;
    for (x=0;x<SIZE;x++) {
        for (y=0;y<SIZE;y++) {
            if (board[x][y]==0) {
                count++;
            }
        }
    }
    return count;
}

bool gameEnded(uint8_t board[SIZE][SIZE]) {
    bool ended = true;
    if (countEmpty(board)>0) return false;
    if (findPairDown(board)) return false;
    rotateBoard(board);
    if (findPairDown(board)) ended = false;
    rotateBoard(board);
    rotateBoard(board);
    rotateBoard(board);
    return ended;
}

void addRandom(uint8_t board[SIZE][SIZE]) {
    static bool initialized = false;
    uint8_t x,y;
    uint8_t r,len=0;
    uint8_t n,list[SIZE*SIZE][2];
    
    if (!initialized) {
        srand(time(NULL));
        initialized = true;
    }
    
    for (x=0;x<SIZE;x++) {
        for (y=0;y<SIZE;y++) {
            if (board[x][y]==0) {
                list[len][0]=x;
                list[len][1]=y;
                len++;
            }
        }
    }
    
    if (len>0) {
        r = rand()%len;
        x = list[r][0];
        y = list[r][1];
        n = (rand()%10)/9+1;
        board[x][y]=n;
    }
}

void initBoard(uint8_t board[SIZE][SIZE]) {
    uint8_t x,y;
    for (x=0;x<SIZE;x++) {
        for (y=0;y<SIZE;y++) {
            board[x][y]=0;
        }
    }
    addRandom(board);
    addRandom(board);
    drawBoard(board);
    score = 0;
}
void initBoard2(uint8_t board[SIZE][SIZE]) {
    uint8_t x,y;
    for (x=0;x<SIZE;x++) {
        for (y=0;y<SIZE;y++) {
            board[x][y]=0;
        }
    }
    addRandom(board);
    addRandom(board);
    drawBoard2(board);
    score = 0;
}

void setBufferedInput(bool enable) {
    static bool enabled = true;
    static struct termios old;
    struct termios new;
    
    if (enable && !enabled) {
        // restore the former settings
        tcsetattr(STDIN_FILENO,TCSANOW,&old);
        // set the new state
        enabled = true;
    } else if (!enable && enabled) {
        // get the terminal settings for standard input
        tcgetattr(STDIN_FILENO,&new);
        // we want to keep the old setting to restore them at the end
        old = new;
        // disable canonical mode (buffered i/o) and local echo
        new.c_lflag &=(~ICANON & ~ECHO);
        // set the new settings immediately
        tcsetattr(STDIN_FILENO,TCSANOW,&new);
        // set the new state
        enabled = false;
    }
}

int test() {
    uint8_t array[SIZE];
    // these are exponents with base 2 (1=2 2=4 3=8)
    uint8_t data[] = {
        0,0,0,1,   1,0,0,0,
        0,0,1,1,   2,0,0,0,
        0,1,0,1,   2,0,0,0,
        1,0,0,1,   2,0,0,0,
        1,0,1,0,   2,0,0,0,
        1,1,1,0,   2,1,0,0,
        1,0,1,1,   2,1,0,0,
        1,1,0,1,   2,1,0,0,
        1,1,1,1,   2,2,0,0,
        2,2,1,1,   3,2,0,0,
        1,1,2,2,   2,3,0,0,
        3,0,1,1,   3,2,0,0,
        2,0,1,1,   2,2,0,0
    };
    uint8_t *in,*out;
    uint8_t t,tests;
    uint8_t i;
    bool success = true;
    
    tests = (sizeof(data)/sizeof(data[0]))/(2*SIZE);
    for (t=0;t<tests;t++) {
        in = data+t*2*SIZE;
        out = in + SIZE;
        for (i=0;i<SIZE;i++) {
            array[i] = in[i];
        }
        slideArray(array);
        for (i=0;i<SIZE;i++) {
            if (array[i] != out[i]) {
                success = false;
            }
        }
        if (success==false) {
            for (i=0;i<SIZE;i++) {
                printf("%d ",in[i]);
            }
            printf("=> ");
            for (i=0;i<SIZE;i++) {
                printf("%d ",array[i]);
            }
            printf("expected ");
            for (i=0;i<SIZE;i++) {
                printf("%d ",in[i]);
            }
            printf("=> ");
            for (i=0;i<SIZE;i++) {
                printf("%d ",out[i]);
            }
            printf("\n");
            break;
        }
    }
    if (success) {
        printf("All %u tests executed successfully\n",tests);
    }
    return !success;
}

void signal_callback_handler(int signum) {
    printf("         TERMINATED         \n");
    setBufferedInput(true);
    printf("\033[?25h\033[m");
    exit(signum);
}

void manual (){
    printf ("\n 시작할 때 사용자가 선택한 사이즈의 판 위에 2개의 2(또는 4)가 있는데 \n");
    printf (" 키보드 방향키나 터치스크린으로 드래그하면 블록이 전부 그 방향으로 이동하면서 \n");
    printf (" 같은 숫자가 있을 경우 합쳐지며 빈 자리 중 한 칸에 랜덤하게 2 또는 4가 나옵니다.\n");
    printf (" 이를 반복해서 2로부터 2048을 만들면 게임 클리어 ! \n");
    printf (" 만약 2048을 만들기 전 이동할 수 없는 경우 즉 16칸이 꽉 차있으면서 인접한 두 칸이 같지 않을 때 게임 오버가 되며,\n");
    printf (" 2048을 만듦과 동시에 이동 불가면 역시 마찬가지 입니다.\n");
    printf (" 참 쉽죠 ? \n");
    printf (" 별 거 아닌 것 같지만...은근히 중독성있고 어렵습니다. \n");
    printf (" 숫자를 뭉칠 때 3개~4개가 한꺼번에 뭉쳐지지는 않습니다. \n");
    printf (" 예를 들어, 4-4-4-4 이렇게 되어 있거나 4-4-8 이렇게 되어 있을 때 뭉치면 8-8로 되고 바로 16으로 되지는 않습니다. \n");
    printf (" 물론 8-8로 된 상태에서 한 번 더 뭉치면 16으로 되는 건 맞습니다. \n ");
    printf (" 즉, 4-4-4-4나 4-4-8 이렇게 된 것을 16으로 뭉치려면 두 번 뭉쳐야 한다는 얘기 ! \n ");
    printf (" 그럼 이제 '플레이'를 하러 가볼까요 ?? \n \n");
}

static inline long myclock()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

int main(int argc, char *argv[]) {
        uint8_t board[SIZE][SIZE];
        uint8_t board2[SIZE][SIZE];
        char c;
        bool success;
        bool success2;
        int start, dis;
        int fd, fd2;
        long t, dt;
        int i ,j, l, player;
        int sle;
        char ranking[100];
        double tem[10];
        double change;
        char record[20];
        char content_write[100];
        char content_read[100];
        char route[100] = "/home/hanhoon/OpenSorce/2048.c/Rank/";
        char name[100];
        int path = access(route, 0);
        goal = false; goal1 = false; goal2 = false;
        
        while(1) {
            
            printf ("2048 게임을 즐길 준비가 되셨나요 ? \n");
            printf ("\n");
            printf ("   1. 플레이 \n");
            printf ("   2. 게임방법 \n");
            printf ("\n");
            
            printf ("원하는 번호를 선택해주세요 : ");
            scanf("%d",&sle);
            
            if(sle == 1) {
                break;
            }
            else
                manual();
        }
        for (i = 1; i <= 5; i++)
        {
            if (path == 0)
            {
                sprintf(name, "%s%d_%d.txt", route, SIZE, i);
                fd = open(name, O_RDWR | O_CREAT);
            }
            
            else if (path == -1)
            {
                int nResult = mkdir(route);
                sprintf(name, "%s%d_%d.txt", route, SIZE, i);
                fd = open(name, O_RDWR | O_CREAT);
            }
            
            fd2 = open(name, O_RDWR | O_CREAT);
            
        }
        printf("\n\n");
        printf("플레이어 수를 입력하세요 (최대 2명) : ");
        scanf("%d",&player);
        printf("\n\n\n\n\n\n\n\n\n\n");
        
        if (argc == 2 && strcmp(argv[1],"test")==0) {
            return test();
        }
        if (argc == 2 && strcmp(argv[1],"blackwhite")==0) {
            scheme = 1;
        }
        if (argc == 2 && strcmp(argv[1],"bluered")==0) {
            scheme = 2;
        }
        printf("\n\n");
        printf("보드의 크기를 입력하세요. ex) 4 : ");
        scanf("%d",&l);
        printf("\n\n\n\n\n\n\n\n\n\n");
        SIZE = l;
        if(SIZE == 0 || SIZE == 1 || SIZE == 2) {
            printf("보드판의 크기가 너무 작아요 !\n");
            exit(1);
        }
        
        
        printf("\033[?25l\033[2J");
        
        // register signal handler for when ctrl-c is pressed
        signal(SIGINT, signal_callback_handler);
        
        initBoard(board);
        if(player == 2) initBoard2(board2);
        setBufferedInput(false);
            if (set_time) {
                start = myclock();
                set_time = false;
            }
            while (true) {
                c=getchar();
                if (c == -1){
                    puts("\nError! Cannot read keyboard input!");
                    break;
                }
                if (goal && goal1) {
                    dis = myclock() - start;
		    printf("\nRecord : %d.%d sec\n", dis / 1000, dis % 1000);
                    
                    printf("\n\n\nWIN\n");
		    printf("\n\n");
                    sprintf(ranking, "%d.%d\n", dis / 1000, dis % 1000);
                    
                    tem[0] = atof(ranking);
                    
                    for (i = 1; i <= 5; i++)
                    {
                        sprintf(name, "%s%d_%d.txt", route, SIZE, i);
                        fd2 = open(name, O_RDWR | O_CREAT);
                        read(fd2, content_read, sizeof(content_read));
                        tem[i] = atof(content_read);
                        if (tem[i] < 1) tem[i] = 10000.0;
                    }
                    for (i = 0; i <= 5; i++)
                    {
                        change = tem[i];
                        
                        for (j = i - 1; j >= 0; j--)
                        {
                            if (tem[j] > change)
                            {
                                tem[j + 1] = tem[j];
                            }
                            else
                                break;
                        }
                        tem[j + 1] = change;
                    }
                    for (i = 1; i <= 5; i++)
                    {
                        sprintf(name, "%s%d_%d.txt", route, SIZE, i);
                        fd = open(name, O_RDWR | O_CREAT);
                        sprintf(content_write, "%f\n", tem[i-1]);
                        write(fd, content_write, sizeof(content_write));
                    }
                    
		    printf("\n");		

                    for (i = 1; i <= 5; i++)
                    {
                        sprintf(name, "%s%d_%d.txt", route, SIZE, i);
                        fd2 = open(name, O_RDWR | O_CREAT);
                        read(fd2, content_read, sizeof(content_read));
                        printf("%d : %s\n",i ,content_read);
                    }
                    break;
                }
                if (goal && goal2) {
                    dis = myclock() - start;
		    printf("\nRecord : %d.%d sec\n", dis / 1000, dis % 1000);
                    
                    printf("\n\n\n\t\t\t\t\t\t\tWIN\n");
		    printf("\n\n");
                    sprintf(ranking, "%d.%d\n", dis / 1000, dis % 1000);
                    
                    tem[0] = atof(ranking);
                    
                    for (i = 1; i <= 5; i++)
                    {
                        sprintf(name, "%s%d_%d.txt", route, SIZE, i);
                        fd2 = open(name, O_RDWR | O_CREAT);
                        read(fd2, content_read, sizeof(content_read));
                        tem[i] = atof(content_read);
                        if (tem[i] < 1) tem[i] = 10000.0;
                    }
                    for (i = 0; i <= 5; i++)
                    {
                        change = tem[i];
                        
                        for (j = i - 1; j >= 0; j--)
                        {
                            if (tem[j] > change)
                            {
                                tem[j + 1] = tem[j];
                            }
                            else
                                break;
                        }
                        tem[j + 1] = change;
                    }
                    for (i = 1; i <= 5; i++)
                    {
                        sprintf(name, "%s%d_%d.txt", route, SIZE, i);
                        fd = open(name, O_RDWR | O_CREAT);
                        sprintf(content_write, "%f\n", tem[i-1]);
                        write(fd, content_write, sizeof(content_write));
                    }
                    
		    printf("\n");

                    for (i = 1; i <= 5; i++)
                    {
                        sprintf(name, "%s%d_%d.txt", route, SIZE, i);
                        fd2 = open(name, O_RDWR | O_CREAT);
                        read(fd2, content_read, sizeof(content_read));
                        printf("%d : %s\n",i ,content_read);
                    }
                    break;
                }
                goal1 = false; goal2 = false;
                switch(c) {
                    case 97:   // 'a' key
                        success = moveLeft(board); success2 =false; goal1 = true;  break;
                    case 68:   // left arrow
                        if(player == 2) {
                            success2 = moveLeft(board2);  success =false; goal2 = true;  break;
                        }
                        
                        
                    case 100:   // 'd' key
                        success = moveRight(board); success2 =false; goal1 = true;   break;
                    case 67:   // right arrow
                        if(player == 2) {
                            success2 = moveRight(board2); success =false; goal2 = true;  break;
                        }
                    case 119:   // 'w' key
                        success = moveUp(board);    success2 =false; goal1 = true;   break;
                    case 65:   // up arrow
                        if(player == 2) {
                            success2 = moveUp(board2);  success =false; goal2 = true;   break;
                        }
                    case 115:   // 's' key
                        success = moveDown(board);  success2 =false; goal1 = true;   break;
                    case 66:   // down arrow
                        if(player == 2) {
                            success2 = moveDown(board2); success =false; goal2 = true;   break;
                        }
                    default: success = false; success2 = false;
                }
                if (success) {
                    drawBoard(board);
                    usleep(150000);
                    addRandom(board);
                    drawBoard(board);
                    if (gameEnded(board)) {
                        dis = myclock() - start;
                        printf("--- %d.%d sec\n", dis / 1000, dis % 1000);
                        
                        printf("         GAME OVER          \n");
                        break;
                    }
                }
                
                if (success2) {
                    drawBoard2(board2);
                    usleep(150000);
                    addRandom(board);
                    drawBoard2(board2);
                    if (gameEnded(board2)) {
                        
                        printf("\t\t\t\t\t\t\t         GAME OVER          \n");
                        break;
                    }
                }
                if(c=='q') {
                    printf("        QUIT? (y/n)         \n");
                    c=getchar();
                    if (c=='y') {
                        break;
                    }
                    drawBoard(board);
                }
                if (c=='r') {
                    printf("       RESTART? (y/n)       \n");
                    c=getchar();
                    if (c=='y') {
                        initBoard(board);
                    }
                    drawBoard(board);
                }
                
                
            }
            setBufferedInput(true);
            
            printf("\033[?25h\033[m");
            close(fd);
            return EXIT_SUCCESS;
    
}
