# Voxel-based-variable-width-continuous-spiral-path-planning-for-3D-printing

**Paper:** 
https://doi.org/10.1016/j.jmapro.2023.10.044

**Abstract:**
Discontinuous printing paths in 3D printing lead to wasted time in fabricating products. This paper proposes a voxel-based continuous spiral path planning algorithm for 3D printing parts with less time consumption. Interestingly, we found our method can also improve the surface qualities and mechanical properties of the printed parts. The proposed path planning strategy in this paper is based on voxels which is different from traditional path planning strategies in literature. Firstly, our proposed algorithm determines the voxel size by considering the printable width of the printer nozzle. It then generates discrete elements by voxelizing the model and creates spirals and line segments by restricting the print direction of the connectable discrete elements. Next, the algorithm employs the Euler circuit to generate continuous sub-paths for the spirals and line segments. It utilizes the depth-first traversal algorithm to traverse and open the continuous sub-paths, resulting in printable continuous paths. To maintain surface accuracy for complex models, contour lines are introduced on the model's exterior and connected internally to ensure global path continuity. Lastly, the algorithm controls the path width in different directions by adjusting the extrusion rate of the printhead, preventing path overlap. Experimental results demonstrate that the proposed algorithm outperforms traditional path planning algorithms and the Connected Fermat spirals algorithm. The proposed strategy effectively mitigates the negative effects of path discontinuity, reduces printing time, and improves printed accuracy, and mechanical properties.


**Note:** 
The library only uploads the core algorithms and input/output results of the articles.

**Contact information:**
Hao Xu (haoxu5640@gmail.com)
