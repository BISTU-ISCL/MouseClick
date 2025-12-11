#include <QApplication>
#include <QDateTime>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <algorithm>
#include <vector>

// 点击记录结构，保存位置与时间戳
struct ClickPoint
{
    QPointF pos;            // 鼠标点击的位置
    qint64 timestamp = 0;   // 点击发生时的毫秒时间戳
};

class ClickWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ClickWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        // 设置窗口标题与初始大小
        setWindowTitle(u8"鼠标点击实时显示");
        setMinimumSize(640, 480);

        // 定时器用于周期性清理过期的点击点并刷新绘制
        m_cleanupTimer.setInterval(50); // 50ms 刷新一次，保证视觉上平滑
        connect(&m_cleanupTimer, &QTimer::timeout, this, [this]() {
            pruneOldClicks();
            update();
        });
        m_cleanupTimer.start();
    }

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        // 记录当前点击位置与时间
        ClickPoint point;
        point.pos = event->position();
        point.timestamp = QDateTime::currentMSecsSinceEpoch();
        m_clicks.push_back(point);

        // 立即刷新显示新的点击
        update();
        QWidget::mousePressEvent(event);
    }

    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        pruneOldClicks();

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const int radius = 8; // 圆点半径

        // 先绘制蓝色圆点（除去最后一个点击点）
        if (!m_clicks.empty())
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(30, 144, 255)); // 道奇蓝
            for (size_t i = 0; i + 1 < m_clicks.size(); ++i)
            {
                const auto &click = m_clicks[i];
                painter.drawEllipse(click.pos, radius, radius);
            }

            // 绘制最后一个点击点为红色实心圆
            painter.setBrush(QColor(220, 20, 60)); // 猩红
            const auto &last = m_clicks.back();
            painter.drawEllipse(last.pos, radius + 2, radius + 2);
        }
    }

private:
    // 删除超过 3 秒的点击记录
    void pruneOldClicks()
    {
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        const qint64 threshold = now - 3000; // 3 秒以内的点击才显示

        auto it = std::remove_if(m_clicks.begin(), m_clicks.end(), [threshold](const ClickPoint &click) {
            return click.timestamp < threshold;
        });
        m_clicks.erase(it, m_clicks.end());
    }

    std::vector<ClickPoint> m_clicks; // 保存最近的点击轨迹
    QTimer m_cleanupTimer;             // 定时器负责清理与刷新
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ClickWidget widget;
    widget.show();

    return app.exec();
}

#include "main.moc"
