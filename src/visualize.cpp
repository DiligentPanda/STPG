// TODO: need to modify the code with the new ADG data structure

// #include <fstream>
// #include <boost/program_options.hpp>
// #include <SFML/Graphics.hpp>
// #include <SFML/System.hpp>
// #include <cmath>
// #include <iostream>
// #include "ADG/generate_ADG.h"
// #include "ADG/ADG_utilities.h"
// #include "nlohmann/json.hpp"

// using json=nlohmann::json;

// // TODO(rivers): organize it in a class

// // // When displaying all 40 agents
// // int COL_SPACE = 41;
// // int ROW_SPACE = 26;
// // int NODE_SIZE = 9;

// int COL_SPACE = 55;
// int ROW_SPACE = 200;
// int NODE_SIZE = 15;

// void sketch_Type1_Simplified_TPG(ADG &adg, std::vector<sf::CircleShape>*& agents_nodes, std::vector<sf::Text>*& labels, sf::Font& font, std::vector<int>& vis_agents){
//     sf::Color colors[] = {sf::Color::Red, sf::Color::Blue, sf::Color::Green, sf::Color::Magenta, sf::Color::Cyan};

//     Paths & paths = *get<1>(adg);
//     //int agent_num = get_agentCnt(adg);
//     std::vector<int> & accum_stateCnts = *get<2>(adg);

//     for (int i = 0; i < int(vis_agents.size()); ++i){
//         int path_len;
//         if (vis_agents[i] == 0)
//             path_len = accum_stateCnts[0];
//         else
//             path_len = accum_stateCnts[vis_agents[i]] - accum_stateCnts[vis_agents[i]-1];

//         for (int j = 0; j < (path_len); ++j){
//             // Define nodes with transparent fill and colored outline
//             sf::CircleShape node1(NODE_SIZE);
//             node1.setFillColor(sf::Color::Transparent);
//             node1.setOutlineThickness(2);
//             node1.setOutlineColor(colors[i % 5]);
//             node1.setPosition(COL_SPACE * j + 4, ROW_SPACE * i);

//             // Labels
//             std::string str_label = std::to_string(paths[vis_agents[i]][j].first.first) + "," + std::to_string(paths[vis_agents[i]][j].first.second);
//             sf::Text label1(str_label.c_str(), font);
//             label1.setCharacterSize(10);
//             label1.setFillColor(sf::Color::Black);
//             // Center the label on the node
//             sf::FloatRect textRect1 = label1.getLocalBounds();
//             label1.setOrigin(textRect1.left + textRect1.width/2.0f, textRect1.top  + textRect1.height/2.0f);
//             label1.setPosition(node1.getPosition() + sf::Vector2f(node1.getRadius(), node1.getRadius()));

//             agents_nodes[i].push_back(node1);
//             labels[i].push_back(label1);
//         }
//     }
// }

// void sketch_Type2_NonSwitchable(ADG& adg, std::vector<sf::CircleShape>*& agents_nodes, std::vector<sf::Text>*& labels, std::vector<int>& agent_states_cnt, std::vector<int>& vis_agents){
//     //int agent_num = get_agentCnt(adg);

//     auto maxIt = std::max_element(agent_states_cnt.begin(), agent_states_cnt.end());

//     for (int i = 0; i < *(maxIt); ++i){
//         for (int a = 0; a < int(vis_agents.size()); ++a){
//             if (i >= agent_states_cnt[a])
//                 continue;

//             std::vector<std::pair<int, int>> outNeighbors_pair = get_nonSwitchable_outNeibPair(adg, vis_agents[a], i);
//             //std::cout << "Starting from node" << i << ", now find agent" << vis_agents[a] << "'s switchable edges..." << std::endl;
            
//             float sketched_starting_pos = agents_nodes[a][i].getPosition().x;
//             float sketched_ending_pos;

//             // Delay nodes
//             for (const std::pair<int, int>& out_pairs: outNeighbors_pair){
//                 auto it = std::find(vis_agents.begin(), vis_agents.end(), out_pairs.first);

//                 if ((it == vis_agents.end()) || (vis_agents[a] == out_pairs.first))
//                     continue;

//                 //std::cout << out_pairs.first << " " << out_pairs.second << std::endl;

//                 int cur_agent = std::distance(vis_agents.begin(), it);
//                 sketched_ending_pos = agents_nodes[cur_agent][out_pairs.second].getPosition().x;
//                 if (sketched_ending_pos <= sketched_starting_pos){
//                     for (int a_s = out_pairs.second; a_s < agent_states_cnt[cur_agent]; ++a_s){
//                         agents_nodes[cur_agent][a_s].setPosition(sketched_starting_pos + COL_SPACE * (a_s - out_pairs.second + 1), ROW_SPACE * cur_agent);
//                         labels[cur_agent][a_s].setPosition(agents_nodes[cur_agent][a_s].getPosition() + sf::Vector2f(agents_nodes[cur_agent][a_s].getRadius(), agents_nodes[cur_agent][a_s].getRadius()));
//                     }
//                 }
//             }
//         }
//     }
// }

// void sketch_Type2_Switchable(ADG& adg, std::vector<sf::CircleShape>*& agents_nodes, std::vector<sf::Text>*& labels, std::vector<int>& agent_states_cnt, std::vector<int>& vis_agents){
//     //int agent_num = get_agentCnt(adg);

//     auto maxIt = std::max_element(agent_states_cnt.begin(), agent_states_cnt.end());

//     for (int i = 0; i < *(maxIt); ++i){
//         for (int a = 0; a < int(vis_agents.size()); ++a){
//             if (i >= agent_states_cnt[a])
//                 continue;

//             std::vector<std::pair<int, int>> outNeighbors_pair = get_switchable_outNeibPair(adg, vis_agents[a], i);
            
//             float sketched_starting_pos = COL_SPACE * i + 4;
//             float sketched_ending_pos;

//             // Delay nodes
//             for (const std::pair<int, int>& out_pairs: outNeighbors_pair){
//                 auto it = std::find(vis_agents.begin(), vis_agents.end(), out_pairs.first);

//                 if ((it == vis_agents.end()) || (vis_agents[a] == out_pairs.first))
//                     continue;

//                 sketched_ending_pos = COL_SPACE * out_pairs.second + 4;
//                 if (sketched_ending_pos <= sketched_starting_pos){
//                     int cur_agent = std::distance(vis_agents.begin(), it);
//                     for (int a_s = out_pairs.second; a_s < agent_states_cnt[cur_agent]; ++a_s){
//                         agents_nodes[cur_agent][a_s].setPosition(agents_nodes[cur_agent][a_s].getPosition() + sf::Vector2f(sketched_starting_pos + COL_SPACE * (- out_pairs.second + 1) - 4, 0));
//                         labels[cur_agent][a_s].setPosition(agents_nodes[cur_agent][a_s].getPosition() + sf::Vector2f(agents_nodes[cur_agent][a_s].getRadius(), agents_nodes[cur_agent][a_s].getRadius()));
//                     }
//                 }
//             }
//         }
//     }
// }

// sf::Vector2f trunc_edge_end(sf::Vector2f start, sf::Vector2f end, float node_r){
//     sf::Vector2f direction = end - start;
//     float ori_len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
//     sf::Vector2f unitDirection = direction / ori_len;
//     end = unitDirection * (ori_len - node_r) + start;
//     return end;
// }

// // Function to draw a directed edge between two nodes
// void drawDirectedEdge(sf::RenderWindow & window, sf::Vector2f start, sf::Vector2f end, const sf::Color& color, bool isDotted, bool directed) {
//     sf::Vector2f direction = end - start;
//     sf::Vector2f unitDirection = direction / std::sqrt(direction.x * direction.x + direction.y * direction.y);
//     sf::Vector2f unitPerpendicular(-unitDirection.y, unitDirection.x);

//     float arrowLength = 10.0f;
//     float arrowWidth = 8.0f;

//     if (!directed){
//         arrowLength = 0.0f;
//         arrowWidth = 0.0f;
//     }

//     if (!isDotted) {
//         sf::Vertex line[] = {
//             sf::Vertex(start, color),
//             sf::Vertex(end, color)
//         };
//         window.draw(line, 2, sf::Lines);
//     } else {
//         // If the edge is dotted, draw it as a series of short segments
//         float segmentLength = 10.0f;
//         float segmentGap = 5.0f;
//         float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
//         int segmentCount = static_cast<int>(length / (segmentLength + segmentGap));

//         for (int i = 0; i < segmentCount; ++i) {
//             sf::Vertex line[] = {
//                 sf::Vertex(start + (direction / length) * (i * (segmentLength + segmentGap)), sf::Color::Black), // Set color to black
//                 sf::Vertex(start + (direction / length) * ((i * (segmentLength + segmentGap)) + segmentLength), sf::Color::Black) // Set color to black
//             };
//             window.draw(line, 2, sf::Lines);
//         }
//     }

//     sf::ConvexShape arrowhead;
//     arrowhead.setPointCount(3);
//     arrowhead.setPoint(0, end);
//     arrowhead.setPoint(1, end - unitDirection * arrowLength + unitPerpendicular * arrowWidth / 2.0f);
//     arrowhead.setPoint(2, end - unitDirection * arrowLength - unitPerpendicular * arrowWidth / 2.0f);
//     arrowhead.setFillColor(color);

//     window.draw(arrowhead);
// }

// void draw(sf::RenderWindow & window, ADG &adg, std::vector<int> &vis_agents, std::vector<int> &agent_states_cnt, std::vector<sf::CircleShape>* & agents_nodes, std::vector<sf::Text>* & labels) {
//     for (int i = 0; i < int(vis_agents.size()); ++i){
//         // Draw nodes and labels
//         for (sf::CircleShape node: agents_nodes[i]){
//             window.draw(node);
//         }
//         for (sf::Text label: labels[i]){
//             window.draw(label);
//         }

//         for (int n = 0; n < int(agents_nodes[i].size() - 1); ++n){
//             // Draw Type 1 Edges
//             //if (time > 10){ //Use codes like this to draw real-time TPG
//             drawDirectedEdge(window, agents_nodes[i][n].getPosition() + sf::Vector2f(agents_nodes[i][n].getRadius() * 2 + 1, agents_nodes[i][n].getRadius()), agents_nodes[i][n+1].getPosition()+ sf::Vector2f(-1, agents_nodes[i][n].getRadius()), sf::Color::Black, false, false);
//             //}
//         }
//         // Example of drawing a directed edge
//     }

//     //---------------------------------------------------
//     // Draw NonSwitchable Type 2
//     auto maxIt = std::max_element(agent_states_cnt.begin(), agent_states_cnt.end());

//     for (int i = 0; i < *(maxIt); ++i){
//         for (int a = 0; a < int(vis_agents.size()); ++a){
//             if (i >= agent_states_cnt[a])
//                 continue;

//             std::vector<std::pair<int, int>> outNeighbors_pair;

//             outNeighbors_pair = get_nonSwitchable_outNeibPair(adg, vis_agents[a], i);

//             for (const std::pair<int, int>& out_pairs: outNeighbors_pair){
//                 auto it = std::find(vis_agents.begin(), vis_agents.end(), out_pairs.first);

//                 if ((it == vis_agents.end()) || (vis_agents[a] == out_pairs.first))
//                     continue;

//                 sf::Vector2f start = agents_nodes[a][i].getPosition() + sf::Vector2f(agents_nodes[a][i].getRadius(), agents_nodes[a][i].getRadius());
//                 sf::Vector2f end = agents_nodes[std::distance(vis_agents.begin(), it)][out_pairs.second].getPosition()+ sf::Vector2f(agents_nodes[a][i].getRadius(), agents_nodes[a][i].getRadius());
//                 end = trunc_edge_end(start, end, agents_nodes[a][i].getRadius());
//                 start = trunc_edge_end(end, start, agents_nodes[a][i].getRadius());
//                 drawDirectedEdge(window, start, end, sf::Color::Black, false, true);
//             }


//             //---------------------------------------------------
//             // Draw Switchable Type 2
//             outNeighbors_pair = get_switchable_outNeibPair(adg, vis_agents[a], i);

//             for (const std::pair<int, int>& out_pairs: outNeighbors_pair){
//                 auto it = std::find(vis_agents.begin(), vis_agents.end(), out_pairs.first);

//                 if ((it == vis_agents.end()) || (vis_agents[a] == out_pairs.first))
//                     continue;
//                 sf::Vector2f start = agents_nodes[a][i].getPosition() + sf::Vector2f(agents_nodes[a][i].getRadius(), agents_nodes[a][i].getRadius());
//                 sf::Vector2f end = agents_nodes[std::distance(vis_agents.begin(), it)][out_pairs.second].getPosition()+ sf::Vector2f(agents_nodes[a][i].getRadius(), agents_nodes[a][i].getRadius());
//                 end = trunc_edge_end(start, end, agents_nodes[a][i].getRadius());
//                 start = trunc_edge_end(end, start, agents_nodes[a][i].getRadius());
//                 drawDirectedEdge(window, start, end, sf::Color::Black, true, true);
//             }
//         }
//     }
// }

// int visualize_ADG(ADG &adg, std::vector<int> &vis_agents) {
//     int agent_num = get_agentCnt(adg);
//     std::cout << "Num of agents: " << agent_num << std::endl;
//     std::vector<int> agent_states_cnt;
//     for (int a: vis_agents){
//         agent_states_cnt.push_back(get_stateCnt(adg, a));
//     }

//     sf::RenderWindow window(sf::VideoMode(500, 500), "Graph Visualization");
//     window.setFramerateLimit(24);

//     sf::Font font;
//     if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf")) {
//         std::cout << "Error loading font\n";
//         exit(-1);
//     }
    
//     std::vector<sf::CircleShape>* agents_nodes = new std::vector<sf::CircleShape>[int(vis_agents.size())];
//     std::vector<sf::Text>* labels = new std::vector<sf::Text>[int(vis_agents.size())];

//     // Create nodes with labels for later drawing. Nodes are stored in `agent_nodes` with labels in `labels`
//     sketch_Type1_Simplified_TPG(adg, agents_nodes, labels, font, vis_agents);
//     // Move some nodes forward according to type-2 edges
//     sketch_Type2_NonSwitchable(adg, agents_nodes, labels, agent_states_cnt, vis_agents);
//     sketch_Type2_Switchable(adg, agents_nodes, labels, agent_states_cnt, vis_agents);

//     // sf::RenderTexture renderTexture;
//     // renderTexture.create(2500, 2500);
//     // renderTexture.clear(sf::Color::White);
//     // draw(renderTexture, adg, vis_agents, agent_states_cnt, agents_nodes, labels);
//     // renderTexture.setSmooth(true);
//     // renderTexture.display();


//     int time = 0;

//     // for view control: https://stackoverflow.com/questions/41788847/dragging-screen-in-sfml
//     bool moving=false;
//     float zoom=1.0f;
//     sf::Vector2f oldPos;
//     // Retrieve the window's default initial view
//     sf::View view = window.getDefaultView();
//     sf::Vector2f base_size=view.getSize();

//     while (window.isOpen()) {
//         sf::Event event;
//         while (window.pollEvent(event)) {
//             if (event.type == sf::Event::Closed){
//                 window.close();
//                 delete[] agents_nodes;
//                 delete[] labels;
//                 std::cout << "DELETED" << std::endl;
//                 return 0;
//             }

//             switch(event.type) {
//                 case sf::Event::Resized: {
//                         // update the view to the new size of the window
//                         base_size.x = event.size.width;
//                         base_size.y = event.size.height;
//                         view.setCenter(view.getCenter()-view.getSize()/2.f+base_size*zoom/2.f);
//                         view.setSize(base_size*zoom);
//                         window.setView(view);
//                         break;
//                     }
//                 case sf::Event::MouseButtonPressed:
//                     if (event.mouseButton.button == sf::Mouse::Left){
//                         moving = true;
//                         oldPos = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
//                     }
//                     break;
//                 case sf::Event::MouseButtonReleased:
//                     if (event.mouseButton.button == sf::Mouse::Left){
//                         moving = false;
//                     }
//                     break;
//                 case sf::Event::MouseMoved: {
//                     if (!moving)
//                         break;
//                     // Determine the new position in world coordinates
//                         const sf::Vector2f newPos = sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
//                         // Determine how the cursor has moved
//                         // Swap these to invert the movement direction
//                         sf::Vector2f deltaPos = oldPos - newPos;

//                         // Applying zoom "reduction" (or "augmentation")
//                         deltaPos.x *= zoom;
//                         deltaPos.y *= zoom;

//                         // Move our view accordingly and update the window
//                         view.move(deltaPos);
//                         window.setView(view);

//                         // Save the new position as the old one
//                         // We're recalculating this, since we've changed the view
//                         oldPos = sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
//                         break;
//                     }
//                 case sf::Event::MouseWheelScrolled: {
//                         // Ignore the mouse wheel unless we're not moving
//                         if (moving)
//                             break;

//                         // Determine the scroll direction and adjust the zoom level
//                         // Again, you can swap these to invert the direction
//                         if (event.mouseWheelScroll.delta <= -1)
//                             zoom = std::min(2.f, zoom + .1f);
//                         else if (event.mouseWheelScroll.delta >= 1)
//                             zoom = std::max(.5f, zoom - .1f);

//                         // accumZoom *= zoom; // <-- accumulate zoom
//                         // Update our view
//                         view.setSize(base_size); // Reset the size
//                         view.zoom(zoom); // Apply the zoom level (this transforms the view)
//                         window.setView(view);
//                         break;
//                     }
//                 default:
//                     break;
//             }




//         }

//         window.clear(sf::Color::White);

//         // const sf::Texture& texture = renderTexture.getTexture();
//         // sf::Sprite sprite(texture);
//         // window.draw(sprite);
//         draw(window, adg, vis_agents, agent_states_cnt, agents_nodes, labels);

//         time ++;
//         //std::cout << time << std::endl;

//         window.display();

//         // Wait or pause for a second (1000 milliseconds) before the next update
//         // sf::sleep(sf::milliseconds(1000));
//     }
//     delete[] agents_nodes;
//     delete[] labels;

//     return 0;
// }

// int main(int argc, char** argv) {

//   namespace po = boost::program_options;
//   po::options_description desc("Switch ADG Optimization");
//   desc.add_options()
//     ("help", "show help message")
//     ("path_fp,p",po::value<std::string>()->required(),"path file to construct ADG")
//     ("sit_fp,s",po::value<std::string>()->required(),"situation file to construct delayed ADG")
//   ;

//   po::variables_map vm;
//   po::store(po::parse_command_line(argc, argv, desc), vm);

//   if (vm.count("help")) {
//       std::cout << desc << std::endl;
//       return 1;
//   }

//   po::notify(vm);

//   string path_fp=vm.at("path_fp").as<string>();
//   string sit_fp=vm.at("sit_fp").as<string>();

//   ADG adg=construct_ADG(path_fp.c_str());
//   ifstream in(sit_fp);
//   json data=json::parse(in);

//   int agent_num=get_agentCnt(adg);

//   vector<int> states=data.at("states").get<vector<int> >();
//   vector<int> delay_steps=data.at("delay_steps").get<vector<int> >();

//   // TODO(rivers): maybe check path_fp with the path_fp saved in sit_fp as well.
//   if ((int)states.size()!=agent_num || (int)delay_steps.size()!=agent_num) {
//     std::cout<<"size mismatch: "<<states.size()<<" "<<delay_steps.size()<<" "<<agent_num<<std::endl;
//     exit(50);
//   }

//   int input_sw_cnt;
//   // construct the delayed ADG by inserting dummy nodes
//   ADG adg_delayed = construct_delayed_ADG(adg, delay_steps, states, input_sw_cnt);

//   std::vector<int> vis_agents;
//   for (int i=0;i<agent_num;++i) {
//     vis_agents.push_back(i);
//   }

//   visualize_ADG(adg, vis_agents);


//   return 0;
// }