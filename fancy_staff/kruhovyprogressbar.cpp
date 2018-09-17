#include "kruhovyprogressbar.h"
#include <QPaintEvent>
#include <QPainter>
#include <QConicalGradient>
#include <QPen>

KruhovyProgressBar::KruhovyProgressBar(QWidget *parent) : QWidget(parent),
    m_loadingAngle(0),
       m_width(0)
{

}
void KruhovyProgressBar::setLoadingAngle(int loadingAngle)
{
    m_loadingAngle = loadingAngle;
}

int KruhovyProgressBar::loadingAngle() const
{
    return m_loadingAngle;
}

void KruhovyProgressBar::setDiscWidth(int width)
{
    m_width = width;
}

int KruhovyProgressBar::discWidth() const
{
    return m_width;
}

void KruhovyProgressBar::paintEvent(QPaintEvent *)
{
    QRect drawingRect;
    drawingRect.setX(rect().x() + m_width);
    drawingRect.setY(rect().y() + m_width);
    drawingRect.setWidth(rect().width() - m_width * 2);
    drawingRect.setHeight(rect().height() - m_width * 2);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QConicalGradient gradient;
    gradient.setCenter(drawingRect.center());
    gradient.setAngle(90);
    gradient.setColorAt(0, QColor(178, 255, 246));
    gradient.setColorAt(1, QColor(5, 44, 50));

    int arcLengthApproximation = m_width + m_width / 3;
    QPen pen(QBrush(gradient), m_width);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    painter.drawArc(drawingRect, 90 * 16 - arcLengthApproximation, -m_loadingAngle * 16);
}
