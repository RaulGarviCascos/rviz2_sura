#include "sensor_panel.hpp"
#include <QVBoxLayout>
#include "rviz_common/visualization_manager.hpp"


SensorPanel::SensorPanel(rviz_common::VisualizationManager * manager, QWidget *parent)
  : QWidget(parent), manager_(manager), current_calculated_columns_(-1) // Inicializamos a -1
{

  QString robot_name = manager_->getRobotConfig().robot_name;

  // Layout principal del panel
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(15, 15, 15, 15);
  main_layout->setSpacing(15);

  // Título del Panel de Sensores
  title_label_= new QLabel(tr("Sensors control panel - %1").arg(robot_name), this);
  title_label_->setAlignment(Qt::AlignCenter);
  QFont font = title_label_->font();
  font.setBold(true);
  font.setPointSize(30);
  title_label_->setFont(font);
  main_layout->addWidget(title_label_);


  // Contenedor interno que tendrá el Grid Layout
  scroll_container_ = new QWidget(this);
  grid_layout_ = new QGridLayout(scroll_container_);
  grid_layout_->setSpacing(15);
  grid_layout_->setContentsMargins(5, 5, 5, 5);
  scroll_container_->setLayout(grid_layout_);

  // Scroll Area responsive
  scroll_area_ = new QScrollArea(this);
  scroll_area_->setWidget(scroll_container_);
  scroll_area_->setWidgetResizable(true);
  scroll_area_->setFrameShape(QFrame::NoFrame);
  scroll_area_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroll_area_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  
  main_layout->addWidget(scroll_area_);
  setLayout(main_layout);

}

Sensor* SensorPanel::addSensor(const QString &name)
{
  if (sensors_map_.contains(name)) {
    return sensors_map_[name]; 
  }

  // 1. Instanciamos la tarjeta del sensor
  Sensor *new_sensor = new Sensor(name, scroll_container_);
  
  // 💡 ESTABLECE LÍMITES: Así evitamos que se deformen en pantallas gigantes o se aplasten en las pequeñas
  new_sensor->setMinimumWidth(220); // Ancho mínimo recomendado para la tarjeta
  new_sensor->setMaximumWidth(350); // Ancho máximo para que no se estire de forma absurda

  sensors_map_.insert(name, new_sensor);
  sensors_list_.append(new_sensor); // Guardamos el orden físico de inserción

  // 2. Conectamos la señal
  connect(new_sensor, &Sensor::sensorToggled, this, [this, name](bool enabled) {
    emit sensorStateChanged(name, enabled);
  });

  // 3. Forzamos un recalculo del grid al añadir un elemento nuevo
  current_calculated_columns_ = -1;
  rearrangeGrid();

  return new_sensor;
}

Sensor* SensorPanel::getSensor(const QString &name) const
{
  return sensors_map_.value(name, nullptr);
}

// =========================================================================
// LÓGICA DE ADAPTACIÓN DINÁMICA
// =========================================================================

void SensorPanel::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  rearrangeGrid(); // Cada vez que RViz o la ventana cambie de tamaño, recalculamos
}

void SensorPanel::rearrangeGrid()
{
  if (sensors_list_.isEmpty()) return;

  // 1. Calculamos el espacio real disponible en el scroll area
  int available_width = scroll_area_->width();
  
  // Anchura objetivo de cada tarjeta + márgenes y espaciados del layout
  int card_width = 220; 
  int spacing = grid_layout_->spacing();
  int margins = grid_layout_->contentsMargins().left() + grid_layout_->contentsMargins().right();

  // 2. Determinamos cuántas columnas caben físicamente en este momento
  int cols = (available_width - margins + spacing) / (card_width + spacing);
  if (cols < 1) cols = 1;

  // Evitamos parpadeos y cálculos innecesarios si el número de columnas no ha cambiado
  if (cols == current_calculated_columns_) return;
  current_calculated_columns_ = cols;

  // 3. Vaciamos el layout actual de forma segura (sin destruir los widgets de las tarjetas)
  QLayoutItem *child;
  while ((child = grid_layout_->takeAt(0)) != nullptr) {
    delete child; // Borra el contenedor del layout item, pero mantiene vivo el widget dentro
  }

  // 4. Distribuimos los sensores secuencialmente en el nuevo número de columnas
  int row = 0;
  int col = 0;
  for (Sensor *sensor : sensors_list_) {
    // Al usar Qt::AlignTop (sin AlignLeft), permitimos que la tarjeta se estire 
    // horizontalmente si le sobra espacio, pero solo hasta su "MaximumWidth" (350px)
    grid_layout_->addWidget(sensor, row, col, Qt::AlignTop);
    
    col++;
    if (col >= cols) {
      col = 0;
      row++;
    }
  }

  // 5. Empujamos todo hacia arriba para que no floten verticalmente si hay pocas filas
  grid_layout_->setRowStretch(row + 1, 1);
  
  // Forzamos a que todas las columnas tengan el mismo peso de estiramiento
  for (int c = 0; c < cols; ++c) {
    grid_layout_->setColumnStretch(c, 1);
  }
}