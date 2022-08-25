# Tension Graph Documentation

This documentation will cover the basic function, purposed and intended use of this plugin.

## Disclaimer:
This plugin requires the use of the **Optimus plugin**, and requires you to use the mesh deformer pipeline instead of the regular deformation pipeline.

It is also intended as an educational tool, and is not designed to be a commercial product. If you have any bugs or issues, please submit them on the GitHub repository below.
https://github.com/JChittockDev/Research/tree/main/TensionGraph

## Intro:
**What exactly is tension?**

Tension is the measurement of force placed on an elastic object. In this case, it is the measuring the force being applied to each vertex. 

**Why is this useful?**

Measuring tension for each vertex gives you access two three variables that are incredibly useful for adding realism to characters, among other utilities.

These variables are:

- Compression (floating point magnitude)
- Stretch (floating point magnitude)
- Direction (directional vector)

**Okay .... but how can I use this?**

In the plugin I have provided a demo scene, using a Metahuman. It uses the compression value to drive a normal map, and the direction vector to blur & sharpen the normals based on their tangent space offset.

If you are interested to see the effects of this implementation, please find the scene called **ExampleLevel** within the plugin content directory, **under the folder called Example.**

## Basic Example Implementation:

- Attach a tension component to an actor instance
- Attach a skeletal mesh to this instance
- Set the skeletal mesh deformer to the deformer graph called **"TensionGraph"**.
- In your skeletal mesh's assigned material, plug the vertex color into your base color slot, and you will be able to see the compression and directional vector changing as the character move and deforms.

## Code Summary:

- **FVertexAdjacencyMapBuffer:**
	- Stores & Initializes a vertex buffer containing vertex adjacency information
- **UTensionComponent:**
	- Base class that inherits from UActorComponent
	- Main purpose is to gather vertex data and pass it to the vertex buffer
- **UTensionDataInterface:**
	- Tension data interface for Optimus framework
- **UTensionDataProvider:**
	- Tension data provider for Optimus framework
- **FTensionDataProviderProxy:**
	- Tension data proxy for optimus framework
- **FTensionGraphModule:**
	- Base plugin registration and constructor/destructor
