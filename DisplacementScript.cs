using UnityEngine;
using System.Collections;

public class DisplacementScript : MonoBehaviour {
	
	// Use this for initialization
	void Start () {
		
		Texture2D tex = (Texture2D)Resources.Load ("_Displacement");
		//		gameObject.GetComponent<Renderer> ().material.SetTexture ("_MainTex", tex);
		Mesh mesh = GetComponent<MeshFilter>().mesh;
		Vector3[] vertices = mesh.vertices;
		Vector2[] uvs = mesh.uv;
		float factor = -0.5f;
		for (int i=0; i<vertices.Length; i++) {
			vertices[i].x += mesh.normals[i].x*tex.GetPixelBilinear(uvs[i].x,uvs[i].y).grayscale*factor;
			vertices[i].y += mesh.normals[i].y*tex.GetPixelBilinear(uvs[i].x,uvs[i].y).grayscale*factor;
			vertices[i].z += mesh.normals[i].z*tex.GetPixelBilinear(uvs[i].x,uvs[i].y).grayscale*factor;
		}
		mesh.vertices = vertices;
		mesh.RecalculateNormals();
		
	}
	
	// Update is called once per frame
	void Update () {
		
	}
}
