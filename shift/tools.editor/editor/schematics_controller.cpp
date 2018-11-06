#include "editor/schematics_controller.hpp"

namespace shift
{
const QString& node_port::name() const
{
  return _name;
}

void node_port::name(QString new_name)
{
  if (_name != new_name)
  {
    _name = std::move(new_name);
    emit nameChanged(_name);
  }
}

schematics_node::schematics_node(QString name) : _name(std::move(name))
{
}

const QString& schematics_node::name() const
{
  return _name;
}

void schematics_node::name(QString new_name)
{
  if (_name != new_name)
  {
    _name = std::move(new_name);
    emit name_changed(_name);
  }
}

const QList<node_port*>& schematics_node::input_ports() const
{
  return _input_ports;
}

const QList<node_port*>& schematics_node::output_ports() const
{
  return _output_ports;
}

add_node::add_node() : schematics_node("add")
{
  _input_operand_a.name("A");
  _input_operand_b.name("B");
  _output.name("output");

  _input_ports.append(&_input_operand_a);
  _input_ports.append(&_input_operand_b);
  emit input_ports_changed(_input_ports);

  _output_ports.append(&_output);
  emit output_ports_changed(_output_ports);
}

schematics_controller::schematics_controller()
{
  _nodes.push_back(std::make_unique<add_node>());
  _nodes.push_back(std::make_unique<add_node>());
  _nodes.push_back(std::make_unique<add_node>());

  for (const auto& node : _nodes)
    _node_model.append(node.get());
}

const QList<QObject*>& schematics_controller::nodeModel() const
{
  return _node_model;
}
}
