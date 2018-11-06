#ifndef SHIFT_SCHEMATICS_CONTROLLER_HPP
#define SHIFT_SCHEMATICS_CONTROLLER_HPP

#include <memory>
#include <QObject>

namespace shift
{
class node_port : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QString name MEMBER _name READ name WRITE name NOTIFY nameChanged)

public:
  ///
  const QString& name() const;

  ///
  void name(QString new_name);

signals:
  void nameChanged(const QString&);

private:
  QString _name;
};

class schematics_node : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QString name MEMBER _name READ name WRITE name NOTIFY name_changed)
  Q_PROPERTY(QList<node_port*> inputPortsModel MEMBER _input_ports READ
               input_ports NOTIFY input_ports_changed)
  Q_PROPERTY(QList<node_port*> outputPortsModel MEMBER _output_ports READ
               output_ports NOTIFY output_ports_changed)

public:
  /// Constructor.
  schematics_node(QString name);

  ///
  const QString& name() const;

  ///
  void name(QString new_name);

  ///
  const QList<node_port*>& input_ports() const;

  ///
  const QList<node_port*>& output_ports() const;

signals:
  void name_changed(const QString&);
  void input_ports_changed(const QList<node_port*>&);
  void output_ports_changed(const QList<node_port*>&);

protected:
  QString _name;
  QList<node_port*> _input_ports;
  QList<node_port*> _output_ports;
};

class add_node : public schematics_node
{
public:
  /// Default constructor.
  add_node();

private:
  node_port _input_operand_a;
  node_port _input_operand_b;
  node_port _output;
};

///
class schematics_controller : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QList<QObject*> nodeModel MEMBER _node_model READ nodeModel NOTIFY
               nodeModelChanged)

public:
  /// Default constructor.
  schematics_controller();

  ///
  const QList<QObject*>& nodeModel() const;

signals:
  void nodeModelChanged(const QList<QObject*>&);

private:
  std::vector<std::unique_ptr<schematics_node>> _nodes;
  QList<QObject*> _node_model;
};
}

#endif
