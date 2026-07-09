#ifndef SURA_BUTTON_HPP
#define SURA_BUTTON_HPP

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

#endif // SURA_BUTTON_HPP