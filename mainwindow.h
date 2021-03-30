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
    Up=1,//����
    Down=2,//����
    qDown=3,//ֱ��
    Left=4,//����
    Right=5//����
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);
    void paintEvent(QPaintEvent *e);//��ͼ
    void keyPressEvent(QKeyEvent *e);//�����¼�
    void moveShape(Move m);//�ƶ��ͱ���
    bool getPointB(int p,int x,int y);//��ȡ��״X,Y�Ƿ��з���
    void setNextShape();//��һ����״
    int rotateShape(int shape);//��ת��״
    bool ifMovable(int shape,int x,int y);//�Ƿ���ƶ�
    void initGame();
    QColor randColor();
    void setRecord();

private:
    Ui::MainWindow *ui;
    int sX,sY,scoreNum,lineNum;//��ǰX,Y����,�÷ּ�����
    QTimer *mainTimer,*secondTimer;//���Ӽ�ʱ��
    bool squareRecords[WIDTH_SHAPE_NUM][HEIGHT_SHAPE_NUM];//ͼ�μ�¼
    QColor colorRecords[WIDTH_SHAPE_NUM][HEIGHT_SHAPE_NUM];//��ɫ��¼
    int curShape,nextShape;//��ǰ��״����һ����״
    QColor curColor,nextColor;//��ǰ��ɫ����һ����ɫ
    bool endGame;

public slots:
    void secondTimerEvent();
    void mainTimerEvent();
};

#endif // MAINWINDOW_H
