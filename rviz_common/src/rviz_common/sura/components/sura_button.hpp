#ifndef RVIZ_COMMON__SURA__COMPONENTS__SURA_BUTTON_HPP_
#define RVIZ_COMMON__SURA__COMPONENTS__SURA_BUTTON_HPP_

#include <QPushButton>

class SuraButton : public QPushButton
{
  Q_OBJECT

public:
  enum class Role {
    Default,  // Azul - Acciones genéricas
    Success,  // Verde - Guardar, aplicar, éxito
    Warning,  // Naranja - Armar motores, modos de vuelo
    Danger,    // Rojo - Parada de emergencia, desarmar
    Run
  };

  explicit SuraButton(Role role, const QString &text, QWidget *parent = nullptr);
  ~SuraButton() override = default;

  // Por si quieres cambiarle el rol dinámicamente en ejecución
  void setRole(Role role);

private:
  Role current_role_;
  void updateStyle();
};

#endif  // RVIZ_COMMON__SURA__COMPONENTS__SURA_BUTTON_HPP_
