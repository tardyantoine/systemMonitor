#include "route_planner.h"
#include <algorithm>

RoutePlanner::RoutePlanner(RouteModel &model, float start_x, float start_y, float end_x, float end_y): m_Model(model) {
    // Convert inputs to percentage:
    start_x *= 0.01;
    start_y *= 0.01;
    end_x *= 0.01;
    end_y *= 0.01;

    start_node = &m_Model.FindClosestNode(start_x, start_y);
    end_node   = &m_Model.FindClosestNode(end_x, end_y);
}

float RoutePlanner::CalculateHValue(RouteModel::Node const *node) {
  return node->distance(*end_node);
}

void RoutePlanner::AddNeighbors(RouteModel::Node *current_node) {
  current_node->FindNeighbors();
  start_node->visited = true;

  for(RouteModel::Node* neighbor : current_node->neighbors) {
    if(!neighbor->visited) {
      neighbor->parent = current_node;
      neighbor->g_value = current_node->g_value + neighbor->distance(*current_node);
      neighbor->h_value = CalculateHValue(neighbor);
      neighbor->visited = true;
      open_list.push_back(neighbor);
    }
  }
}

bool CompGH(RouteModel::Node* node1, RouteModel::Node* node2) {
  return node1->g_value + node1->h_value > node2->g_value + node2->h_value;
}

RouteModel::Node *RoutePlanner::NextNode() {
  std::sort(open_list.begin(), open_list.end(), CompGH);

  auto out = open_list.back();
  open_list.pop_back();
  return out; 
}

std::vector<RouteModel::Node> RoutePlanner::ConstructFinalPath(RouteModel::Node *current_node) {
    // Create path_found vector
    distance = 0.0f;
    std::vector<RouteModel::Node> path_found;
    end_node = current_node;
    path_found.emplace(path_found.begin(), *end_node);

    while(current_node != start_node) {
      distance += current_node->distance(*current_node->parent);
      path_found.emplace(path_found.begin(), *current_node->parent);
      current_node = current_node->parent;
    }

    distance *= m_Model.MetricScale(); // Multiply the distance by the scale of the map to get meters.
    return path_found;

}

void RoutePlanner::AStarSearch() {
    RouteModel::Node *current_node = nullptr;

  AddNeighbors(start_node);

  while(open_list.size() > 0) {
    current_node = NextNode();
    if(current_node == end_node) {
      std::cout << "Found end node, displaying path..." << std::endl; 
      m_Model.path = ConstructFinalPath(current_node);
      return;
    }
    AddNeighbors(current_node);
  }

  std::cout << "No path found, exiting." << std::endl;
}