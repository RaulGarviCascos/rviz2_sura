#include "settings_panel.hpp"
#include "rviz_common/visualization_frame.hpp"
#include "rviz_common/visualization_manager.hpp"
#include <QVBoxLayout>
#include "../components/sura_button.hpp" 
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QIcon>
#include <QPixmap>
#define CONFIG_EXTENSION "rviz"
#define CONFIG_EXTENSION_WILDCARD "*." CONFIG_EXTENSION

SettingsPanel::SettingsPanel(rviz_common::VisualizationFrame * frame, QWidget *parent)
  : QWidget(parent), frame_(frame)
{
  
  RobotConfig r_config = frame_->getManager()->getRobotConfig();
  QString robot_name = r_config.robot_name;
  QString user_name =  r_config.user;
  QString robot_ip = r_config.ip;
  QString user_pass= r_config.password;
  QString file_path = r_config.file_path;
  int width_panel = 1000;

  QVBoxLayout *main_layout = new QVBoxLayout(this);
  title_label_ = new QLabel(tr("Settings panel - %1").arg(robot_name), this);
  title_label_->setAlignment(Qt::AlignCenter);
  QFont font = title_label_->font();
  font.setBold(true);
  font.setPointSize(30);
  title_label_->setFont(font);
  main_layout->addWidget(title_label_);

  QWidget *scroll_widget = new QWidget(this);
  settings_grid_ = new QGridLayout(scroll_widget);
  settings_grid_->setVerticalSpacing(20);
  scroll_widget->setLayout(settings_grid_);

  QFont title_font = this->font(); 
  title_font.setBold(true);
  title_font.setPointSize(15);

  QFont text_font = this->font();
  text_font.setPointSize(8);

  // ==========================================================
  // CAJA GENERAL CONTENEDORA (Ocupa la fila 0 del layout principal)
  // ==========================================================
  QGroupBox *general_box = new QGroupBox(scroll_widget);  
  QVBoxLayout *general_layout = new QVBoxLayout(general_box);
  general_layout->setSpacing(15);            
  general_layout->setContentsMargins(15, 20, 15, 25); 

  general_box->setStyleSheet(
    "QGroupBox {"
    "  background-color: #f9f9f9;"     // Gris muy claro de fondo general
    "  border: 1px solid #d0d0d0;"     // Contorno exterior
    "  border-radius: 8px;"
    "}"
  );
  general_box->setFixedWidth(600); 
  // ==========================================================
  // BLOQUE 1: PROJECT SETTINGS (Fila 0 del layout principal)
  // ==========================================================
  QGroupBox *box_project_settings = new QGroupBox(general_box);
  QGridLayout *grid_project_settings = new QGridLayout(box_project_settings);
  grid_project_settings->setContentsMargins(15, 18, 15, 15);

  // Fila 0 interna: Título
  QLabel *title_project_settings = new QLabel(tr("Project Settings"), box_project_settings);
  title_project_settings->setAlignment(Qt::AlignTop);
  title_project_settings->setAlignment(Qt::AlignCenter);
  title_project_settings->setFont(title_font);
  grid_project_settings->addWidget(title_project_settings, 0, 0, 1, 3); // Ocupa 1 fila y 3 columnas
  grid_project_settings->setVerticalSpacing(12);

  QLabel *lbl_ip = new QLabel(tr("Robot ip:"),box_project_settings);
  ip_robot_ = new QLineEdit(tr("%1").arg(robot_ip));
  grid_project_settings->addWidget(lbl_ip, 1, 0);  // Fila 1, Columna 0
  grid_project_settings->addWidget(ip_robot_, 1, 1); // Fila 1, Columna 1

  // QPushButton *folder_button = new QPushButton(tr("Folder"), box_project_settings);
  // connect(folder_button, &QPushButton::clicked, this, &SettingsPanel::onSelectWorkspace);
  // grid_project_settings->addWidget(folder_button,1,2);  
  // grid_project_settings->setVerticalSpacing(12);
  get_file_button_ = new SuraButton(SuraButton::Role::Default, tr("Get xacro"), this);
  get_file_button_->setCheckable(true);
  get_file_button_->setFixedWidth(568);
  connect(get_file_button_, &SuraButton::clicked, this, &SettingsPanel::onGetFileClicked);
  grid_project_settings->addWidget(get_file_button_,1,2);  
  grid_project_settings->setVerticalSpacing(12);

  QLabel *lbl_ws = new QLabel(tr("Xacro file:"), box_project_settings);
  description_file_= new QLineEdit(tr("%1").arg(file_path),box_project_settings);
  grid_project_settings->addWidget(lbl_ws, 2, 0); 
  grid_project_settings->addWidget(description_file_, 2, 1); 
  
  QLabel *lbl_robot_name = new QLabel(tr("Robot Name:"), box_project_settings);
  input_name_ = new QLineEdit(tr("%1").arg(robot_name),box_project_settings);
  grid_project_settings->addWidget(lbl_robot_name, 3, 0);  
  grid_project_settings->addWidget(input_name_, 3, 1); 

  QLabel *lbl_user_name = new QLabel(tr("Pi User Name:"), box_project_settings);
  user_name_ = new QLineEdit(tr("%1").arg(user_name),box_project_settings);
  grid_project_settings->addWidget(lbl_user_name, 4, 0);  
  grid_project_settings->addWidget(user_name_, 4, 1); 

  QLabel *lbl_pass = new QLabel(tr("Password:"), box_project_settings);
  input_pass_ = new QLineEdit(tr("%1").arg(user_pass),box_project_settings);
  input_pass_->setEchoMode(QLineEdit::Password);
  input_pass_->setPlaceholderText(tr("Enter FTP password..."));
  QString normal_color  = "#3498db"; 
  QString hover_color   = "#2980b9"; 
  QString pressed_color = "#1c638e";
  QPushButton *eye_button = new QPushButton("",box_project_settings);
  QPixmap pixmap_eye_opened = rviz_common::loadPixmap("package://rviz_common/icons/opened_eye.svg");
  QPixmap pixmap_eye_closed = rviz_common::loadPixmap("package://rviz_common/icons/closed_eye.svg");
  opened_eye_icon = new QIcon(pixmap_eye_opened);
  closed_eye_icon = new QIcon(pixmap_eye_closed);
  eye_button->setStyleSheet(
    QString(
      "QPushButton {"
      "    background-color: %1;"
      "    font-weight: bold;"
      "    color:white;"
      "    border: none;"
      "    border-radius: 4px;"
      "    padding: 6px 14px;"
      "    min-width: 90px;"
      "}"
      "QPushButton:hover {"
      "    background-color: %2;"
      "}"
      "QPushButton:pressed {"
      "    background-color: %3;"
      "}"
      "QPushButton:disabled {"
      "    background-color: #bdc3c7;"
      "    color: #7f8c8d;"
      "}"
    ).arg(normal_color, hover_color, pressed_color)
  );
  eye_button->setIcon(*opened_eye_icon);
  
  connect(eye_button, &QPushButton::clicked, this, [this, eye_button]() {
    if (input_pass_->echoMode() == QLineEdit::Password) {
      input_pass_->setEchoMode(QLineEdit::Normal);  
      eye_button->setIcon(*closed_eye_icon);              
    } else {
      input_pass_->setEchoMode(QLineEdit::Password); 
      eye_button->setIcon(*opened_eye_icon);             
    }
  });

  grid_project_settings->addWidget(lbl_pass, 5, 0);  
  grid_project_settings->addWidget(input_pass_, 5, 1);
  grid_project_settings->addWidget(eye_button, 5, 2);

  grid_project_settings->setColumnStretch(1, 1);
  grid_project_settings->setRowStretch(6, 1);
  box_project_settings->setMaximumWidth(width_panel);

  box_project_settings->setStyleSheet(
    "QGroupBox { background-color: #ffffff; border: 1px solid #e0e0e0; border-radius: 6px; }"
  );
  box_project_settings->setFixedWidth(568);


  general_layout->addWidget(box_project_settings, 0, Qt::AlignHCenter);

  // ==========================================================
  // LÍNEA DIVISORIA (Fila 1 del layout principal)
  // ==========================================================
  QFrame *line = new QFrame(general_box);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  line->setMaximumWidth(width_panel);
  
  // Añadimos la línea justo debajo de la primera caja
  general_layout->addWidget(line, 0, Qt::AlignHCenter);

  // ==========================================================
  // BLOQUE 2: SURA SETTINGS (Fila 2 del layout principal)
  // ==========================================================
  QGroupBox *box_sura_settings = new QGroupBox(general_box);
  QGridLayout *grid_sura_settings = new QGridLayout(box_sura_settings);
  grid_sura_settings->setContentsMargins(15, 18, 15, 15);
  // Fila 0 interna: Título (Reutilizando title_font)
  QLabel *title_sura_settings = new QLabel(tr("Sura Settings"), box_sura_settings);
  title_sura_settings->setAlignment(Qt::AlignTop);
  title_sura_settings->setAlignment(Qt::AlignCenter);
  title_sura_settings->setFont(title_font);
  grid_sura_settings->addWidget(title_sura_settings, 0, 0, 1, 2);
  grid_sura_settings->setVerticalSpacing(12);
  QCheckBox *check_teleop = new QCheckBox(tr("Dark theme"), box_sura_settings);
  grid_sura_settings->addWidget(check_teleop, 1, 0, 1, 2);
  grid_sura_settings->setVerticalSpacing(12);
  // Fila 1 interna: Ejemplo de un Desplegable (Selector de Modo)
  QLabel *lbl_language = new QLabel(tr("Language:"), box_sura_settings);
  QComboBox *language = new QComboBox(box_sura_settings);
  language->addItems({"English", "Spanish", "Chinese"});
  grid_sura_settings->addWidget(lbl_language, 2, 0);
  grid_sura_settings->addWidget(language, 2, 1);
  grid_sura_settings->setColumnStretch(1,1);
  grid_sura_settings->setRowStretch(3, 1);
  box_sura_settings->setMaximumWidth(width_panel);
  box_sura_settings->setStyleSheet(
    "QGroupBox { background-color: #ffffff; border: 1px solid #e0e0e0; border-radius: 6px; }"
  );
  box_sura_settings->setFixedWidth(568);
  general_layout->addWidget(box_sura_settings, 0, Qt::AlignHCenter);

 
  btn_save_ = new SuraButton(SuraButton::Role::Success, tr("Save"), this);
  btn_save_->setCheckable(true);
  btn_save_->setFixedWidth(568);
  connect(btn_save_, &SuraButton::clicked, this, &SettingsPanel::onSaveClicked);

  general_layout->addWidget(btn_save_, 0, Qt::AlignHCenter);

  // La añadimos al layout general
  settings_grid_->addWidget(general_box, 0, 0, Qt::AlignHCenter | Qt::AlignTop);
  settings_grid_->setRowStretch(1, 1);

  // Añadimos la segunda caja al layout principal (Fila 2)
  scroll_area_ = new QScrollArea(this);
  scroll_area_->setWidget(scroll_widget);
  scroll_area_->setWidgetResizable(true); 
  scroll_area_->setFrameShape(QFrame::NoFrame); 
  scroll_area_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroll_area_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  main_layout->addWidget(scroll_area_);
  setLayout(main_layout);
}



void SettingsPanel::onSelectWorkspace()
{
  // Abrimos el selector de carpetas
  QString dir_path = QFileDialog::getExistingDirectory(
    this, 
    tr("Select Workspace Directory"),                 // Título de la ventana
    QString::fromStdString("./"),                     // Ruta inicial por defecto
    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks // Opciones
  );

  // Si el usuario no ha cancelado la ventana...
  if (!dir_path.isEmpty()) {
    // Aquí dir_path ya contiene la ruta absoluta (ej: "/home/raul/mi_workspace")
    description_file_->setText(dir_path); 
    RCLCPP_INFO(rclcpp::get_logger("rviz2"), "Dir changed to: %s",dir_path.toStdString().c_str());

  }
}

void SettingsPanel::onSaveClicked(){
  QString new_robot_name = input_name_->text();
  QString ip = ip_robot_->text();
  QString user = user_name_->text();
  QString pass = input_pass_->text();
  title_label_->setText(tr("Settings panel - %1").arg(new_robot_name));
  QString file_path=description_file_->text();
   if (frame_) {
    RobotConfig r_config;
    r_config.robot_name=new_robot_name;
    r_config.ip=ip;
    r_config.user = user;
    r_config.password = pass;
    r_config.file_path = file_path;
    frame_->saveNewRobotConfig(r_config);
  }
}


void SettingsPanel::onGetFileClicked(){
  QString string_ip_robot = ip_robot_->text();
  QString string_robot_name = input_name_->text();
  QString string_pass = input_pass_->text();
  QString string_user_name = user_name_->text();
  QString file_name = tr("%1.urdf.xacro").arg(string_robot_name);
  QString string_description_file = (description_file_->text().contains(file_name))?description_file_->text():tr("%1/%2.urdf.xacro").arg(description_file_->text(),string_robot_name);
  RCLCPP_INFO(rclcpp::get_logger("rviz2"), "Trying to get the file: %s", string_description_file.toStdString().c_str());
  if (!string_ip_robot.isEmpty() && !string_robot_name.isEmpty() && 
      !string_pass.isEmpty() && !string_user_name.isEmpty() && !string_description_file.isEmpty()) {
    
    QString target_dir = QDir::homePath() + "/.cirtesu/xacros";

    // 2. Creamos la estructura de carpetas si no existe
    QDir dir;
    if (!dir.mkpath(target_dir)) {
      RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "Could not create directory path: %s", target_dir.toStdString().c_str());
      QMessageBox::critical(
        this, 
        tr("Directory Error"), 
        tr("Failed to create folder path on:\n%1").arg(target_dir)
      );
      return;
    }

    local_output_path_ = target_dir + "/" + string_robot_name + ".urdf.xacro";
    if (local_output_path_.isEmpty()) {
      return;
    }
    frame_->setLocalPathXacro(local_output_path_);
    QString path_to_save = local_output_path_;
    QProcess *ftp_process = new QProcess(this);
    QStringList arguments;
    arguments << "--connect-timeout" << "10" 
              << "-k"
              << "-u" << QString("%1:%2").arg(string_user_name, string_pass)
              << QString("sftp://%1/%2").arg(string_ip_robot, string_description_file)
              << "-o" << local_output_path_;

    
    connect(ftp_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, ftp_process, path_to_save](int exitCode, QProcess::ExitStatus exitStatus) {
        
        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
          // ¡Éxito total!
          RCLCPP_INFO(rclcpp::get_logger("rviz2"), "FTP Download finished successfully: %s", path_to_save.toStdString().c_str());
          QMessageBox::information(
            this, 
            tr("Download Successful"), 
            tr("The description file has been downloaded successfully to:\n%1").arg(path_to_save)
          );
          emit xacroUpdated();
        } else {
          QString error_output = ftp_process->readAllStandardError();
          RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "SFTP Download failed: %s", error_output.toStdString().c_str());
          QMessageBox::critical(
            this, 
            tr("Download Failed"), 
            tr("Could not fetch the file from the robot.\n\nError Log:\n%1").arg(error_output.isEmpty() ? "Timeout or host unreachable" : error_output)
          );
        }
        
        ftp_process->deleteLater();
    });
    ftp_process->start("curl", arguments);
    
    return;

  } else {
    QMessageBox::warning(
      this, 
      tr("Missing Fields"), 
      tr("Please, ensure all configuration fields (IP, Name, User, Password, File and Workspace) are filled before fetching.") 
    );
    return;
  }
}