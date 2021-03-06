#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QTimer>
#include <QPainter>
#include <QKeyEvent>
#include <QMessageBox>
#include <QDebug>
#include <time.h>
#include <string>
#include <QStack>

const int SIDE_LEN=20;
const int HEIGHT_SHAPE_NUM=20;
const int WIDTH_SHAPE_NUM=10;
const int INIT_X=3;
const int INIT_Y=-1;
const int SHAPE_NUM=7;
const int COLOR_NUM=4;
const int SHAPES[7]={0x4444,0x4460,0x2260,0x0C60,0x06C0,0x0660,0x04E0};
const int COLORS[COLOR_NUM]={51,102,153,204};

using namespace std;

namespace Ui {
    class MainWindow;
}

enum Move{
    Up=1,//变形
    Down=2,//加速
    qDown=3,//直落
    Left=4,//左移
    Right=5//右移
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);
    void paintEvent(QPaintEvent *e);//绘图
    void keyPressEvent(QKeyEvent *e);//按键事件
    void moveShape(Move m);//移动和变形
    bool getPointB(int p,int x,int y);//获取形状X,Y是否有方块
    void setNextShape();//下一个形状
    int rotateShape(int shape);//旋转形状
    bool ifMovable(int shape,int x,int y);//是否可移动
    void initGame();
    QColor randColor();
    void setRecord();

private:
    Ui::MainWindow *ui;
    int sX,sY,scoreNum,lineNum;//当前X,Y坐标,得分及行数
    QTimer *mainTimer,*secondTimer;//主从计时器
    bool squareRecords[WIDTH_SHAPE_NUM][HEIGHT_SHAPE_NUM];//图形记录
    QColor colorRecords[WIDTH_SHAPE_NUM][HEIGHT_SHAPE_NUM];//颜色记录
    int curShape,nextShape;//当前形状和下一个形状
    QColor curColor,nextColor;//当前颜色和下一个颜色
    bool endGame;

public slots:
    void secondTimerEvent();
    void mainTimerEvent();
};

#endif // MAINWINDOW_H
