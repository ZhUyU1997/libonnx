#include <onnx.h>

struct operator_pdata_t {
	float alpha;
};

static void LeakyRelu_init(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat;
	struct onnx_tensor_t * t = n->inputs[0];
	int i;

	for(i = 0; i < n->noutput; i++)
	{
		if(n->outputs[i]->type == ONNX_TENSOR_TYPE_UNDEFINED)
			onnx_tensor_reinit(n->outputs[i], t->type, t->dims, t->ndim);
	}

	pdat = malloc(sizeof(struct operator_pdata_t));
	if(pdat)
		pdat->alpha = onnx_attribute_read_float(n, "alpha", 0.01);
	n->priv = pdat;
}

static void LeakyRelu_exit(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;

	if(pdat)
		free(pdat);
}

static void LeakyRelu_float16(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	uint16_t * px = (uint16_t *)x->datas;
	uint16_t * py = (uint16_t *)y->datas;
	float v;
	int i, l;

	for(i = 0, l = y->ndata; i < l; i++)
	{
		v = float16_to_float32(px[i]);
		if(v < 0)
			v *= pdat->alpha;
		py[i] = float32_to_float16(v);
	}
}

static void LeakyRelu_float32(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	float * px = (float *)x->datas;
	float * py = (float *)y->datas;
	int i, l;

	for(i = 0, l = y->ndata; i < l; i++)
		py[i] = (px[i] < 0) ? px[i] * pdat->alpha : px[i];
}

static void LeakyRelu_float64(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	double * px = (double *)x->datas;
	double * py = (double *)y->datas;
	int i, l;

	for(i = 0, l = y->ndata; i < l; i++)
		py[i] = (px[i] < 0) ? px[i] * pdat->alpha : px[i];
}

void default_resolver_op_LeakyRelu(struct onnx_node_t * n)
{
	switch(n->inputs[0]->type)
	{
	case ONNX_TENSOR_TYPE_FLOAT16:
		n->init = LeakyRelu_init;
		n->exit = LeakyRelu_exit;
		n->op = LeakyRelu_float16;
		break;
	case ONNX_TENSOR_TYPE_FLOAT32:
		n->init = LeakyRelu_init;
		n->exit = LeakyRelu_exit;
		n->op = LeakyRelu_float32;
		break;
	case ONNX_TENSOR_TYPE_FLOAT64:
		n->init = LeakyRelu_init;
		n->exit = LeakyRelu_exit;
		n->op = LeakyRelu_float64;
		break;
	default:
		break;
	}
}
