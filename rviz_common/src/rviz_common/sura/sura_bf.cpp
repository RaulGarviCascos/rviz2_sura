#include "rviz_common/sura/sura_bf.hpp"
#include "rviz_common/visualization_frame.hpp"
#include "rclcpp/rclcpp.hpp"
#include "thrusters_panel.hpp"

SuraBF::SuraBF(rviz_common::VisualizationFrame * frame,  QObject * parent)
: QObject(parent),
  frame_(frame),
  tab_widget_(nullptr),
  sensors_tab_(nullptr),
  graphics_tab_(nullptr),
  Thrusters_tab_(nullptr),
  settings_tab_(nullptr),
  rviz_3d_tab_(nullptr)
{
}

SuraBF::~SuraBF()
{
  // Qt se encarga de eliminar los widgets hijos automáticamente si tienen parent,
  // pero si tab_widget no se añadió a ningún layout, lo borramos manualmente.
  if (tab_widget_ && !tab_widget_->parentWidget()) {
    delete tab_widget_;
  }
}

void SuraBF::initSuraUi(QWidget * rviz_render_panel)
{
  if (tab_widget_) return;

  tab_widget_ = new QTabWidget(frame_);

  sensors_tab_ = createSensorsWidget();
  graphics_tab_ = createGraphicsWidget();
  Thrusters_tab_ = createThrustersWidget();
  settings_tab_ = createSettingsWidget();
  rviz_3d_tab_ = createRviz3DWidget(rviz_render_panel);

  tab_widget_->addTab(sensors_tab_, tr("Sensors"));
  tab_widget_->addTab(graphics_tab_, tr("Graphics"));
  tab_widget_->addTab(Thrusters_tab_, tr("Thursters"));
  tab_widget_->addTab(settings_tab_, tr("Settings"));
  tab_widget_->addTab(rviz_3d_tab_, tr("3D View"));
  connect(tab_widget_, &QTabWidget::currentChanged, this, [this](int index) {
    emit tabChanged(index);
  });
  tab_widget_->setVisible(true);
}

void SuraBF::setVisible(bool visible)
{
  if (tab_widget_) {
    tab_widget_->setVisible(visible);
  }
}

QWidget * SuraBF::createSensorsWidget()
{
  QWidget * widget = new QWidget(tab_widget_);
  QVBoxLayout * layout = new QVBoxLayout(widget);

  QLabel * label = new QLabel(tr("Sensores"), widget);
  label->setAlignment(Qt::AlignCenter);
  
  layout->addWidget(label);
  widget->setLayout(layout);
  return widget;
}

QWidget * SuraBF::createGraphicsWidget()
{
  QWidget * widget = new QWidget(tab_widget_);
  QVBoxLayout * layout = new QVBoxLayout(widget);

  QLabel * label = new QLabel(tr("Gráficas"), widget);
  label->setAlignment(Qt::AlignCenter);

  layout->addWidget(label);
  widget->setLayout(layout);
  return widget;
}

QWidget * SuraBF::createThrustersWidget()
{
  ThrustersPanel * Thrusters_panel = new ThrustersPanel(frame_->getManager(),tab_widget_);
  
  return Thrusters_panel;
}

QWidget * SuraBF::createSettingsWidget()
{
  QWidget * widget = new QWidget(tab_widget_);
  QVBoxLayout * layout = new QVBoxLayout(widget);

  QLabel * label = new QLabel(tr("Configuración"), widget);
  label->setAlignment(Qt::AlignCenter);

  layout->addWidget(label);
  widget->setLayout(layout);
  return widget;
}

QWidget * SuraBF::createRviz3DWidget(QWidget * rviz_render_panel)
{
  QWidget * widget = new QWidget(tab_widget_);
  QVBoxLayout * layout = new QVBoxLayout(widget);
  layout->setContentsMargins(0, 0, 0, 0); 

  if (rviz_render_panel) {
    rviz_render_panel->setParent(widget);
    layout->addWidget(rviz_render_panel);
  } else {
    QLabel * label = new QLabel(tr("Error: No se ha podido cargar la Vista 3D nativa"), widget);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
  }

  widget->setLayout(layout);
  return widget;
}