#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(3*WS_PX + (WIDTH_SHAPE_NUM+4)*SIDE_LEN, 2*WS_PX + HEIGHT_SHAPE_NUM*SIDE_LEN);
    secondTimer=new QTimer(this);
    secondTimer->start(50);
    connect(secondTimer,SIGNAL(timeout()),SLOT(secondTimerEvent()));
    secondTimer->stop();
    mainTimer=new QTimer(this);
    mainTimer->start(1000);
    connect(mainTimer,SIGNAL(timeout()),SLOT(mainTimerEvent()));
    mainTimer->stop();
    qsrand(clock());
    nextColor = randColor();
    initGame();
    overStep = -10;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete mainTimer;
    delete secondTimer;
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

void drawRect(QPainter& painter, QRect& rect, QColor& color) {
    painter.fillRect(rect, color);

    painter.setPen(QColor(color.red()+0x33, color.green()+0x33, color.blue()+0x33));
    painter.drawLine(rect.topLeft(), rect.topRight()); // 上连线
    painter.drawLine(rect.topLeft(), rect.bottomLeft()); // 左边线

    painter.setPen(QColor(color.red()-0x33, color.green()-0x33, color.blue()-0x33));
    painter.drawLine(rect.topRight(), rect.bottomRight()); // 右边线
    painter.drawLine(rect.bottomLeft(), rect.bottomRight()); // 下边线
}

void MainWindow::paintEvent(QPaintEvent *e){
    QPainter painter(this);
    int x,y;
    QRect mainRect(WS_PX-1,WS_PX-1,SIDE_LEN*WIDTH_SHAPE_NUM+1,SIDE_LEN*HEIGHT_SHAPE_NUM+1);
    QRect nextRect(2*WS_PX+WIDTH_SHAPE_NUM*SIDE_LEN-1,WS_PX-1,SIDE_LEN*4+1,SIDE_LEN*4+1);
    painter.fillRect(0, 0, width(), height(), QColor(255,255,255));
    painter.setPen(QColor(0x33,0x33,0x33));
    painter.drawRect(mainRect); // 主游戏窗
    painter.drawRect(nextRect); // 下一个形状窗

    painter.setFont(QFont(QString(""), 16, 100, false));

    y = WS_PX + painter.fontInfo().pixelSize() / 2;
    {
        QString strs[] = {QString("分数：%1").arg(scoreNum), QString("行数：%1").arg(lineNum)};
        QPointF p(nextRect.left(), nextRect.bottom() + WS_PX);

        painter.setPen(curColor);
        for(int i=0; i<sizeof(strs)/sizeof(QString); i++) {
            p.setY(p.y() + y);
            painter.drawText(p, strs[i]);
        }
    }

    y = WS_PX + painter.fontInfo().pixelSize() / 2;
    {
        QString strs[] = {
            QString("变形：⇈"),
            QString("左移：⇇"),
            QString("右移：⇉"),
            QString("下落：⇊"),
            QString("落下：空格"),
            QString("新建：F6"),
            QString("暂停：F5"),
        };
        QPointF p(nextRect.left(), mainRect.bottom() + WS_PX);

        painter.setPen(QColor(0xff,0x22,0x00));
        for(int i=sizeof(strs)/sizeof(QString)-1; i>=0; i--) {
            p.setY(p.y() - y);
            painter.drawText(p, strs[i]);
        }
    }

    {
        QPointF p(nextRect.left() + nextRect.width()/2, mainRect.top() + mainRect.height()/2);

        painter.setPen(Qt::red);
        painter.setFont(QFont(QString(""), 80, 100, false));
        p.setX(p.x() - painter.fontInfo().pixelSize() / 2);
        p.setY(p.y() + painter.fontInfo().pixelSize() / 2);
        painter.drawText(p, QString("囍"));
    }

    // 下一个形状
    for(y=0;y<4;y++)
        for(x=0;x<4;x++){
            if(x+sX>=0 && y+sY>=0 && x+sX<WIDTH_SHAPE_NUM && y+sY<HEIGHT_SHAPE_NUM && getPointB(curShape,x,y)){
                QRect rect(WS_PX+(x+sX)*SIDE_LEN,WS_PX+(y+sY)*SIDE_LEN,SIDE_LEN,SIDE_LEN);
                ::drawRect(painter, rect, curColor);
            }
            if(getPointB(nextShape,x,y)){
                QRect rect(2*WS_PX+(x+WIDTH_SHAPE_NUM)*SIDE_LEN,WS_PX+y*SIDE_LEN,SIDE_LEN,SIDE_LEN);
                ::drawRect(painter, rect, nextColor);
            }
        }

    // 主游戏窗
    for(y=0;y<HEIGHT_SHAPE_NUM;y++)
        for(x=0;x<WIDTH_SHAPE_NUM;x++)
            if(squareRecords[x][y]){
                QRect rect(WS_PX+x*SIDE_LEN,WS_PX+y*SIDE_LEN,SIDE_LEN,SIDE_LEN);
                ::drawRect(painter, rect, colorRecords[x][y]);
            }

    // 游戏结束
    if(endGame){
        QString str("游戏结束！");
        QRect rect(painter.boundingRect(mainRect, Qt::AlignCenter, str));

        int alpha = overColor.alpha() + overStep;
        if(overStep > 0) {
            if(alpha > 0xff) {
                alpha = 0xff;
                overStep = -overStep;
            }
        } else {
            if(alpha < 0) {
                alpha = 0;
                overStep = -overStep;
                overColor = randColor();
            }
        }
        overColor.setAlpha(alpha);
        painter.setPen(overColor);
        painter.setFont(QFont(QString(""), 40, 100, false));
        painter.drawText(mainRect, Qt::AlignCenter, str, &rect);
    }
}

void MainWindow::secondTimerEvent()
{
    if(endGame) repaint();
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
    if(!mainTimer->isActive() || endGame) {return;}
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
        secondTimer->start();
        endGame=true;
        overColor = randColor();
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
    secondTimer->stop();
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
