#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    secondTimer=new QTimer;
    secondTimer->start(1000);
    connect(secondTimer,SIGNAL(timeout()),SLOT(secondTimerEvent()));
    mainTimer=new QTimer;
    mainTimer->start(1000);
    connect(mainTimer,SIGNAL(timeout()),SLOT(mainTimerEvent()));
    mainTimer->stop();
    qsrand(time(0));
    initGame();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete mainTimer;
    delete secondTimer;
    delete squareRecords;
    delete colorRecords;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::paintEvent(QPaintEvent *e){
    QPainter painter(this);
    int x,y;
    painter.setPen(QColor(102,102,102));
    painter.drawRect(20,20,SIDE_LEN*WIDTH_SHAPE_NUM,SIDE_LEN*HEIGHT_SHAPE_NUM);
    painter.drawRect(40+WIDTH_SHAPE_NUM*SIDE_LEN,20,SIDE_LEN*4,SIDE_LEN*4);

    painter.drawText(QPoint(40+WIDTH_SHAPE_NUM*SIDE_LEN,40+SIDE_LEN*4),QString("Scores:%1,Lines:%2").arg(scoreNum).arg(lineNum));

    for(y=0;y<4;y++)
        for(x=0;x<4;x++){
            if(x+sX>=0 && y+sY>=0 && x+sX<WIDTH_SHAPE_NUM && y+sY<HEIGHT_SHAPE_NUM && getPointB(curShape,x,y)){
                painter.setBrush(curColor);
                painter.drawRect(20+(x+sX)*SIDE_LEN,20+(y+sY)*SIDE_LEN,SIDE_LEN,SIDE_LEN);
            }
            if(getPointB(nextShape,x,y)){
                painter.setBrush(nextColor);
                painter.drawRect(40+(x+WIDTH_SHAPE_NUM)*SIDE_LEN,20+y*SIDE_LEN,SIDE_LEN,SIDE_LEN);
            }
        }
    for(y=0;y<HEIGHT_SHAPE_NUM;y++)
        for(x=0;x<WIDTH_SHAPE_NUM;x++)
            if(squareRecords[x][y]){
                painter.setBrush(colorRecords[x][y]);
                painter.drawRect(20+x*SIDE_LEN,20+y*SIDE_LEN,SIDE_LEN,SIDE_LEN);
            }
    if(endGame){
        painter.setPen(Qt::red);
        painter.setFont(QFont(QString(""),20,100));
        painter.drawText(QPoint(60,200),QString("Game Over!"));
    }
}

void MainWindow::secondTimerEvent()
{
}
void MainWindow::mainTimerEvent()
{
    moveShape(Down);
    repaint();
}
void MainWindow::keyPressEvent(QKeyEvent *e){
    switch(e->key()){
    case(Qt::Key_Up):
        moveShape(Up);
        break;
    case(Qt::Key_Down):
        moveShape(Down);
        break;
    case(Qt::Key_Space):
        moveShape(qDown);
        break;
    case(Qt::Key_Left):
        moveShape(Left);
        break;
    case(Qt::Key_Right):
        moveShape(Right);
        break;
    case(Qt::Key_F5):
        if(endGame) break;
        if(mainTimer->isActive()){
            mainTimer->stop();
        }else{
            mainTimer->start();
        }
        break;
    case(Qt::Key_F6):
        initGame();
        setNextShape();
        mainTimer->start();
        repaint();
        break;
    }
}

bool MainWindow::getPointB(int p,int x,int y){
 return (p&(1<<(16-(x+1+y*4))));
}

void MainWindow::moveShape(Move m){
    if(!mainTimer->isActive() || endGame) return;
    switch(m){
    case(Up):
        if(ifMovable(rotateShape(curShape),sX,sY))
            curShape=rotateShape(curShape);
        break;
    case(Down):
        if(ifMovable(curShape,sX,sY+1)){
            sY++;
        }else{
            setRecord();
            setNextShape();
        }
        break;
    case(qDown):
        while(ifMovable(curShape,sX,sY+1))
            sY++;
        setRecord();
        setNextShape();
        break;
    case(Left):
        if(ifMovable(curShape,sX-1,sY))
            sX--;
        break;
    case(Right):
        if(ifMovable(curShape,sX+1,sY))
            sX++;
        break;
    }
    repaint();
}
void MainWindow::setNextShape(){
    if(endGame) return;
    sX=INIT_X;sY=INIT_Y;
    if(!ifMovable(nextShape,sX,sY)){
        curShape=0;
        mainTimer->stop();
        endGame=true;
        repaint();
    }
    curShape=nextShape;
    nextShape=SHAPES[qrand()%SHAPE_NUM];
    curColor=nextColor;
    nextColor=randColor();
    if(mainTimer->interval()!=int(1000-scoreNum*0.5))
        mainTimer->setInterval(int(1000-scoreNum*0.5));
}
int MainWindow::rotateShape(int shape){
    int s=0;
    for(int y=0;y<4;y++)
        for(int x=0;x<4;x++)
            if(getPointB(shape,4-y-1,x))
                s|=(1<<(16-(x+1+y*4)));
    return s;
}
bool MainWindow::ifMovable(int shape,int X,int Y){
    for(int y=0;y<4;y++)
        for(int x=0;x<4;x++)
            if(getPointB(shape,x,y))
                if(X+x<0 || X+x>=WIDTH_SHAPE_NUM || Y+y>=HEIGHT_SHAPE_NUM || (Y+y>=0 && squareRecords[X+x][Y+y]))
                    return false;
    return true;
}
void MainWindow::initGame(){
    endGame=false;
    curShape=0;
    nextShape=SHAPES[qrand()%SHAPE_NUM];
    nextColor=randColor();
    sX=INIT_X;
    sY=INIT_Y;scoreNum=0;lineNum=0;
    for(int y=0;y<HEIGHT_SHAPE_NUM;y++)
        for(int x=0;x<WIDTH_SHAPE_NUM;x++)
            squareRecords[x][y]=false;
}
QColor MainWindow::randColor(){
    int r,g,b;
    r=qrand()%COLOR_NUM;
    g=qrand()%COLOR_NUM;
    b=qrand()%COLOR_NUM;
    while(r==g && r==b){
        switch(qrand()%3){
        case(0):
            r=qrand()%COLOR_NUM;
            break;
        case(1):
            g=qrand()%COLOR_NUM;
            break;
        case(2):
            b=qrand()%COLOR_NUM;
            break;
        }
    }
    return QColor(COLORS[r],COLORS[g],COLORS[b]);
}
void MainWindow::setRecord(){
    int x,y,_x,_y,size=0;
    bool flag;
    QStack<int> stack;

    for(y=0;y<4;y++)
        for(x=0;x<4;x++){
            if(getPointB(curShape,x,y)){
                squareRecords[sX+x][sY+y]=true;
                colorRecords[sX+x][sY+y]=curColor;
                flag=true;
                for(_x=0;_x<WIDTH_SHAPE_NUM;_x++)
                    if(!squareRecords[_x][sY+y]){
                        flag=false;
                        break;
                    }
                if(flag)
                    stack.push(sY+y);
            }
        }
    if(stack.size()){
        scoreNum+=stack.size()*2-1;
        lineNum+=stack.count();
        y=stack.pop();
        size=1;
        do{
            while(!stack.isEmpty() && y-size==stack.last()){
                size++;
                qDebug()<<stack.last();
                _y=stack.pop();
            }
            for(x=0;x<WIDTH_SHAPE_NUM;x++){
                squareRecords[x][y]=squareRecords[x][y-size];
                colorRecords[x][y]=colorRecords[x][y-size];
            }
            y--;
        }while(y-size>=0);
    }
}
