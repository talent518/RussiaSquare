#include <iostream>
#include <QTime>
#include <QDesktopWidget>
#include <cmath>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    resize(3*WS_PX + (WIDTH_SHAPE_NUM+4)*SIDE_LEN, 2*WS_PX + HEIGHT_SHAPE_NUM*SIDE_LEN);
    setFixedSize(width(), height());

    {
        QDesktopWidget *dsk = QApplication::desktop();
        QRect rect = dsk->availableGeometry();
        move(rect.x() + (rect.width() - width()) / 2, rect.y() + (rect.height() - height()) /2);
    }

    secondTimer=new QTimer(this);
    secondTimer->start(50);
    connect(secondTimer,SIGNAL(timeout()),SLOT(secondTimerEvent()));
    mainTimer=new QTimer(this);
    mainTimer->start(1000);
    connect(mainTimer,SIGNAL(timeout()),SLOT(mainTimerEvent()));
    mainTimer->stop();
    qsrand(clock());
    nextColor = randColor();
    initGame();
    overStep = -5;
    overTop = 0;
    debugBall = false;
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

void drawRect(QPainter& painter, QRect& _rect, QColor& color) {
    QRectF rect(_rect.left()+0.5f, _rect.top()+0.5f, _rect.width()-1.0f, _rect.height()-1.0f);

    painter.fillRect(rect, color);

    painter.setPen(QColor(color.red()+0x33, color.green()+0x33, color.blue()+0x33));
    painter.drawLine(rect.topLeft(), rect.topRight()); // 上连线
    painter.drawLine(rect.topLeft(), rect.bottomLeft()); // 左边线

    painter.setPen(QColor(color.red()-0x33, color.green()-0x33, color.blue()-0x33));
    painter.drawLine(rect.topRight(), rect.bottomRight()); // 右边线
    painter.drawLine(rect.bottomLeft(), rect.bottomRight()); // 下边线
}

QColor notColor(QColor color) {
    return QColor(0xff-color.red(), 0xff-color.green(), 0xff-color.blue());
}

void MainWindow::paintEvent(QPaintEvent *e){
    QPainter painter(this);
    int x,y;
    QRect mainRect(WS_PX-1,WS_PX-1,SIDE_LEN*WIDTH_SHAPE_NUM+1,SIDE_LEN*HEIGHT_SHAPE_NUM+1);
    QRect nextRect(2*WS_PX+WIDTH_SHAPE_NUM*SIDE_LEN-1,WS_PX-1,SIDE_LEN*4+1,SIDE_LEN*4+1);

    painter.fillRect(0, 0, width(), height(), QColor(0xff,0xff,0xff));
    painter.setPen(QColor(0x66,0x66,0x66));
    painter.drawRect(mainRect); // 主游戏窗边框
    painter.drawRect(nextRect); // 下一个形状窗边框

    // 下一个形状方块
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

    // 主游戏窗方块
    for(y=0;y<HEIGHT_SHAPE_NUM;y++)
        for(x=0;x<WIDTH_SHAPE_NUM;x++)
            if(squareRecords[y][x]){
                QRect rect(WS_PX+x*SIDE_LEN,WS_PX+y*SIDE_LEN,SIDE_LEN,SIDE_LEN);
                ::drawRect(painter, rect, colorRecords[y][x]);
            }

    // 防止锯齿
    painter.setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform);

    // 绘制分数和行数
    y = WS_PX + painter.fontInfo().pixelSize() / 2;
    {
        QString strs[] = {QString("分数：%1").arg(scoreNum), QString("行数：%1").arg(lineNum)};
        QPointF p(nextRect.left(), nextRect.bottom() + WS_PX);

        painter.setPen(QColor(0xff,0x22,0x00));
        painter.setFont(QFont(QString(""), 12, 100, false));
        for(int i=0; i < (int)(sizeof(strs)/sizeof(QString)); i++) {
            p.setY(p.y() + y);
            painter.drawText(p, strs[i]);
        }
    }

    // 绘制帮助
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

    // 绘制时钟
    {
        QRect rect(nextRect.x(), SIDE_LEN*(HEIGHT_SHAPE_NUM-4)/2, nextRect.width(), nextRect.height());
        QPointF cp = rect.center();
        int overAngle = rect.width() / 2;
        QTime t = QTime::currentTime();
        float sRad = (90.0f - t.second() * 6.0f) * 3.14159265f / 180.0f;
        float mRad = (90.0f - t.minute() * 6.0f - 6.0f * t.second() / 60.0f) * 3.14159265f / 180.0f;
        float hRad = (90.0f - t.hour() * 30.0f - 30.0f * t.minute() / 60.0f) * 3.14159265f / 180.0f;

        QPen pen(QColor(0x33,0x33,0x33));

        // 表盘
        pen.setWidth(3);
        painter.setPen(pen);
        painter.drawEllipse(rect);
        painter.setFont(QFont(QString(""), 8, 100, false));
        for(int i=0; i<360; i+=6) {
            float rad = (90.0f - i) * 3.14159265f / 180.0f;
            QPointF p1 = cp + QPointF(overAngle * 0.9f * std::cos(rad), - overAngle * 0.9f * std::sin(rad));
            QPointF p2 = cp + QPointF((overAngle - 1.5f) * std::cos(rad), - (overAngle - 1.5f) * std::sin(rad));
            pen.setWidth(i%30==0 ? (i%90==0 ? 4 : 2) : 1); // cout << i << ": " << pen.width() << endl;
            painter.setPen(pen);
            painter.drawLine(p1, p2);
            if(pen.width() > 1) {
                QPointF p3 = cp + QPointF(overAngle * 0.78f * std::cos(rad), - overAngle * 0.78f * std::sin(rad));
                QString str = QString("%1").arg(i == 0 ? 12 : i/30);
                p3.setX(p3.x()-painter.fontInfo().pixelSize()*str.length()/3.0f+1.5f);
                p3.setY(p3.y()+painter.fontInfo().pixelSize()/3.0f+1.0f);
                painter.drawText(p3, str);
            }

        }

        pen.setColor(Qt::black);

        // 时针
        pen.setWidth(3);
        painter.setPen(pen);
        painter.drawLine(cp, cp + QPointF(overAngle * 0.5f * std::cos(hRad), - overAngle * 0.5f * std::sin(hRad)));

        // 分针
        pen.setWidth(2);
        painter.setPen(pen);
        painter.drawLine(cp, cp + QPointF(overAngle * 0.65f * std::cos(mRad), - overAngle * 0.65f * std::sin(mRad)));

        pen.setColor(Qt::red);

        // 秒针
        pen.setWidth(1);
        painter.setPen(pen);
        painter.drawLine(cp, cp + QPointF(overAngle * 0.7f * std::cos(sRad), - overAngle * 0.7f * std::sin(sRad)));

        // 数字时间
        {
            QString str(QString("%1:%2:%3").arg(t.hour(),2,10,QChar('0')).arg(t.minute(),2,10,QChar('0')).arg(t.second(),2,10,QChar('0')));
            QRect rect1(rect.x(), rect.bottom() + WS_PX, rect.width(), SIDE_LEN);
            QRect rect2(painter.boundingRect(rect1, Qt::AlignCenter, str));

            painter.setFont(QFont(QString(""), 14, 100, false));
            painter.drawText(rect1, Qt::AlignCenter, str, &rect2);
        }
    }

    // 游戏结束/暂停
    if(endGame || pauseGame){
        QString str(endGame ? "游戏结束" : "游戏暂停");

        paintBall(painter);

        overTop += overStep;
        if(overStep > 0) {
            if(overTop > 0xff) {
                overTop = 0xff;
                overStep = -overStep;
                overBackColor = randColor();
                overFrontColor = notColor(overBackColor);
            }
        } else {
            if(overTop < -0xff) {
                overTop = -0xff;
                overStep = -overStep;
                overBackColor = randColor();
                overFrontColor = notColor(overBackColor);
            }
        }
        mainRect.moveTop(mainRect.y() + overTop);

        QRect rect(painter.boundingRect(mainRect, Qt::AlignCenter, str));

        painter.setPen(overBackColor);
        painter.setFont(QFont(QString(""), SIDE_LEN*1.5, 200, false));
        painter.drawText(mainRect, Qt::AlignCenter, str, &rect);

        painter.setPen(overFrontColor);
        painter.setFont(QFont(QString(""), SIDE_LEN*1.5, 175, false));
        painter.drawText(mainRect, Qt::AlignCenter, str, &rect);
    }
}

void MainWindow::secondTimerEvent()
{
    repaint();
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
            pauseGame = true;
            overBackColor = randColor();
            overFrontColor = notColor(overBackColor);
            initBall();
        }else{
            mainTimer->start();
            pauseGame = false;
        }
        break;
    case(Qt::Key_F6):
        initGame();
        setNextShape();
        mainTimer->start();
        repaint();
        break;
    case(Qt::Key_D):
        if(e->modifiers() & Qt::ControlModifier) {
            debugBall = !debugBall; // 弹球调试线开关：Ctrl+D
        }
        break;
    default:
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
        // std::cout << "down" << std::endl;
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
    if(sY<0) goto end;

    {
        int x, y, mY = 0;
        for(y=3; y>=0; y--) {
            for(x=0; x<4; x++) {
                if(getPointB(nextShape, x, y)) {
                    mY = std::max(mY, y);
                    break;
                }
            }
        }
        sX=(WIDTH_SHAPE_NUM-4)/2;sY=-mY-1;
        // std::cout << "mY: " << mY << ", sX: " << sX << ", sY: " << sY << std::endl;
    }

    if(!ifMovable(nextShape,sX,sY+1)){
        end:
        curShape=0;
        mainTimer->stop();
        endGame=true;
        overStep=-5;
        overTop=0;
        overBackColor = randColor();
        overFrontColor = notColor(overBackColor);
        initBall();
        repaint();
        return;
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
                if(X+x<0 || X+x>=WIDTH_SHAPE_NUM || Y+y>=HEIGHT_SHAPE_NUM || (Y+y>=0 && squareRecords[Y+y][X+x]))
                    return false;
    return true;
}
void MainWindow::initGame(){
    endGame=false;
    pauseGame=false;
    sX = sY = 0;
    curShape=0;
    nextShape=SHAPES[qrand()%SHAPE_NUM];
    nextColor=randColor();
    scoreNum=0;lineNum=0;
    for(int y=0;y<HEIGHT_SHAPE_NUM;y++)
        for(int x=0;x<WIDTH_SHAPE_NUM;x++)
            squareRecords[y][x]=false;
}
QColor MainWindow::randColor(){
    int r,g,b;
    r=qrand()%COLOR_NUM;
    g=qrand()%COLOR_NUM;
    b=qrand()%COLOR_NUM;
    return QColor(COLORS[r],COLORS[g],COLORS[b]);
}
void MainWindow::setRecord(){
    int x,y,size=0;
    bool flag;
    QStack<int> stack;

    for(y=0;y<4;y++) {
        if(sY+y<0) continue;

        for(x=0;x<4;x++){
            if(getPointB(curShape,x,y)){
                squareRecords[sY+y][sX+x]=true;
                colorRecords[sY+y][sX+x]=curColor;
            }
        }

        flag=true;
        for(x=0;x<WIDTH_SHAPE_NUM;x++) {
            if(!squareRecords[sY+y][x]){
                flag=false;
                break;
            }
        }
        if(flag) {
            stack.push(sY+y);
        }
    }
    if(stack.size()){
        scoreNum+=stack.size()*2-1;
        lineNum+=stack.count();

#if 0
        std::cout << stack.size() << ':';
        for(y=0;y<stack.size();y++)
            std::cout << ' ' << stack.value(y);
        std::cout << std::endl;
#endif

        y=stack.pop();
        size=1;
        do{
            while(!stack.isEmpty() && y-size==stack.last()){
                size++;
                stack.pop();
            }
            for(x=0;x<WIDTH_SHAPE_NUM;x++){
                squareRecords[y][x]=squareRecords[y-size][x];
                colorRecords[y][x]=colorRecords[y-size][x];
            }
            y--;
        }while(y-size>=0);
        for(;y>=0;y--) {
            for(x=0;x<WIDTH_SHAPE_NUM;x++) {
                squareRecords[y][x]=false;
            }
        }
    }
}

void MainWindow::initBall() {
    const int width = WIDTH_SHAPE_NUM * SIDE_LEN - SIDE_LEN;
    const int height = HEIGHT_SHAPE_NUM * SIDE_LEN - SIDE_LEN;
    overPoint1.setX(qrand() % width);
    overPoint1.setY(qrand() % height);
    do {
        overAngle = qrand() % 360;
    } while((int)overAngle % 90 < 15);
    overRadius = 0.0f;
    overBall = 0;
    overPoint3 = overPoint1;
    overPoint4 = overPoint1;
    calcBall();
}

void MainWindow::calcBall() {
    const double width = WIDTH_SHAPE_NUM * SIDE_LEN - SIDE_LEN;
    const double height = HEIGHT_SHAPE_NUM * SIDE_LEN - SIDE_LEN;

    if(overAngle < 90.0f) {
        overAngle2 = std::atan(overPoint1.y() / (width - overPoint1.x())) * 180.0f / PI;
        if(overAngle > overAngle2) {
            overMaxRadius = overPoint1.y() / std::cos((90.0f - overAngle) * PI / 180.0f);
            overPoint2.setX(overPoint1.x() + overMaxRadius * std::cos(overAngle * PI / 180.0f));
            overPoint2.setY(0);
        } else {
            overMaxRadius = (width - overPoint1.x()) / std::cos(overAngle * PI / 180.0f);
            overPoint2.setX(width);
            overPoint2.setY(overPoint1.y() - overMaxRadius * std::sin(overAngle * PI / 180.0f));
        }
    } else if(overAngle < 180) {
        overAngle2 = 180.0f - std::atan(overPoint1.y() / overPoint1.x()) * 180.0f / PI;
        if(overAngle > overAngle2) {
            overMaxRadius = overPoint1.x() / std::cos((180.0f-overAngle) * PI / 180.0f);
            overPoint2.setX(0);
            overPoint2.setY(overPoint1.y() - overMaxRadius * std::sin(overAngle * PI / 180.0f));
        } else {
            overMaxRadius = overPoint1.y() / std::cos((overAngle - 90.0f) * PI / 180.0f);
            overPoint2.setX(overPoint1.x() + overMaxRadius * std::cos(overAngle * PI / 180.0f));
            overPoint2.setY(0);
        }
    } else if(overAngle < 270) {
        overAngle2 = 270.0f - std::atan(overPoint1.x() / (height - overPoint1.y())) * 180.0f / PI;
        if(overAngle > overAngle2) {
            overMaxRadius = (height - overPoint1.y()) / std::cos((270.0f - overAngle) * PI / 180.0f);
            overPoint2.setX(overPoint1.x() + overMaxRadius * std::cos(overAngle * PI / 180.0f));
            overPoint2.setY(height);
        } else {
            overMaxRadius = overPoint1.x() / std::cos((overAngle-180.0f) * PI / 180.0f);
            overPoint2.setX(0);
            overPoint2.setY(overPoint1.y() - overMaxRadius * std::sin(overAngle * PI / 180.0f));
        }
    } else {
        overAngle2 = 360.0f - std::atan((height - overPoint1.y()) / (width - overPoint1.x())) * 180.0f / PI;
        if(overAngle > overAngle2) {
            overMaxRadius = (width - overPoint1.x()) / std::cos((360.0f - overAngle) * PI / 180.0f);
            overPoint2.setX(width);
            overPoint2.setY(overPoint1.y() - overMaxRadius * std::sin(overAngle * PI / 180.0f));
        } else {
            overMaxRadius = (height - overPoint1.y()) / std::cos((overAngle - 270.0f) * PI / 180.0f);
            overPoint2.setX(overPoint1.x() + overMaxRadius * std::cos(overAngle * PI / 180.0f));
            overPoint2.setY(height);
        }
    }

    overBallColor = randColor();
    overBall ++;
    if(overBall > 99) overBall = 1;

   //  cout << overBall << ": overAngle = " << overAngle << ", overAngle2 = " << overAngle2 << ", overMaxRadius = " << overMaxRadius << endl;
}

void MainWindow::paintBall(QPainter& painter) {
    double x, y;
    overRadius += 10.0f;
    if(overRadius >= overMaxRadius) {
        overRadius = 0.0f;
        overPoint4 = overPoint3;
        overPoint3 = overPoint1;
        overPoint1 = overPoint2;
        if(overAngle < 90.0f) {
            if(overAngle > overAngle2) {
                overAngle = 360.0f - (90.0f - (90.0f - overAngle));
            } else {
                overAngle = (90.0f - overAngle) + 90.0f;
            }
        } else if(overAngle < 180.0f) {
            if(overAngle > overAngle2) {
                overAngle = 90.0f - (90.0f - (180.0f - overAngle));
            } else {
                overAngle = 90.0f - (overAngle - 90.0f) + 180.0f;
            }
        } else if(overAngle < 270.0f) {
            if(overAngle > overAngle2) {
                overAngle = 180.0f - (90.0f - (270.0f - overAngle));
            } else {
                overAngle = 270.0f + (90.0f - (overAngle - 180.0f));
            }
        } else {
            if(overAngle > overAngle2) {
                overAngle = 270.0f - (90.0f - (360.0f - overAngle));
            } else {
                overAngle = 90.0f - (overAngle - 270.0f);
            }
        }
        calcBall();
        x = overPoint1.x();
        y = overPoint1.y();
    } else {
        x = overPoint1.x() + overRadius * std::cos(overAngle * PI / 180);
        y = overPoint1.y() - overRadius * std::sin(overAngle * PI / 180);
    }

    if(debugBall) {
        painter.setPen(QColor(overBallColor.red()-0x33, overBallColor.green()-0x33, overBallColor.blue()-0x33, 0x33));
        painter.setBrush(QColor(overBallColor.red(), overBallColor.green(), overBallColor.blue(), 0x33));
        painter.drawEllipse(QRectF(WS_PX + overPoint4.x(), WS_PX + overPoint4.y(), SIDE_LEN, SIDE_LEN));

        painter.setPen(QColor(overBallColor.red()-0x33, overBallColor.green()-0x33, overBallColor.blue()-0x33, 0x66));
        painter.setBrush(QColor(overBallColor.red(), overBallColor.green(), overBallColor.blue(), 0x66));
        painter.drawEllipse(QRectF(WS_PX + overPoint3.x(), WS_PX + overPoint3.y(), SIDE_LEN, SIDE_LEN));

        painter.setPen(QColor(overBallColor.red()-0x33, overBallColor.green()-0x33, overBallColor.blue()-0x33, 0x99));
        painter.setBrush(QColor(overBallColor.red(), overBallColor.green(), overBallColor.blue(), 0x99));
        painter.drawEllipse(QRectF(WS_PX + overPoint1.x(), WS_PX + overPoint1.y(), SIDE_LEN, SIDE_LEN));

        painter.setPen(QColor(overBallColor.red()-0x33, overBallColor.green()-0x33, overBallColor.blue()-0x33, 0xcc));
        painter.setBrush(QColor(overBallColor.red(), overBallColor.green(), overBallColor.blue(), 0xcc));
        painter.drawEllipse(QRectF(WS_PX + overPoint2.x(), WS_PX + overPoint2.y(), SIDE_LEN, SIDE_LEN));

        QPainterPath path;
        path.moveTo(overPoint4.x() + WS_PX + SIDE_LEN / 2.0f, overPoint4.y() + WS_PX + SIDE_LEN / 2.0f);
        path.lineTo(overPoint3.x() + WS_PX + SIDE_LEN / 2.0f, overPoint3.y() + WS_PX + SIDE_LEN / 2.0f);
        path.lineTo(overPoint1.x() + WS_PX + SIDE_LEN / 2.0f, overPoint1.y() + WS_PX + SIDE_LEN / 2.0f);
        path.lineTo(overPoint2.x() + WS_PX + SIDE_LEN / 2.0f, overPoint2.y() + WS_PX + SIDE_LEN / 2.0f);
        painter.setPen(overBallColor);
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
    }

    painter.setPen(Qt::NoPen);

    painter.setBrush(overBallColor);
    QRectF rect(WS_PX + x, WS_PX + y, SIDE_LEN, SIDE_LEN);
    painter.drawEllipse(rect);

    painter.setFont(QFont(QString(""), SIDE_LEN/2, 100, false));
    QString str = QString("%1").arg(overBall);
    QRectF rect2(painter.boundingRect(rect, Qt::AlignCenter, str));
    painter.setPen(Qt::white);
    rect.moveTop(rect.y() + 3.0f);
    painter.drawText(rect, Qt::AlignCenter, str, &rect2);
}
