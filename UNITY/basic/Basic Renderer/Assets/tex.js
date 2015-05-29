#pragma strict
//using UnityEditor;
function Start () {
	var mesh : Mesh = GetComponent.<MeshFilter>().mesh;
	var disp : Texture2D =  Resources.Load("068depthmap2_4") as Texture2D;
	var verts : Vector3[] = mesh.vertices;
	var uvs : Vector2[] = mesh.uv;
	//Debug.Log(mesh.vertices.length);
	var size=verts.length;
	for(var i=0;i<size;i++)
	{
		verts[i] =  verts[i] + mesh.normals[i] * disp.GetPixelBilinear(uvs[i].x,uvs[i].y).grayscale * -1.0;
	}
	mesh.vertices=verts;
	mesh.RecalculateNormals();
}

function Update () {
}