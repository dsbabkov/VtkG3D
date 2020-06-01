#pragma once

#include <vtkMappedDataArray.h>
#include "Mesh.h"

class VtkNodes: public vtkMappedDataArray<float>
{
	vtkMappedDataArrayNewInstanceMacro(VtkNodes)
public:
	static VtkNodes *New();
	vtkTypeBool Allocate(vtkIdType size, vtkIdType ext = 1000) override {
		return true;
		assert("Not implemented" == nullptr);
	}
	vtkTypeBool Resize(vtkIdType numTuples) override {
		return true;
		assert("Not implemented" == nullptr);
	}

	void GetTuples(vtkIdList *ptIds, vtkAbstractArray *output) override {
		assert("Not implemented" == nullptr);
	}
	void GetTuples(vtkIdType p1, vtkIdType p2, vtkAbstractArray *output) override {
		assert("Not implemented" == nullptr);
	}
	void DeepCopy(vtkAbstractArray *aa) override {
		assert("Not implemented" == nullptr);
	}
	void DeepCopy(vtkDataArray *da) override {
		assert("Not implemented" == nullptr);
	}
	void InterpolateTuple(vtkIdType i, vtkIdList *ptIndices,
			vtkAbstractArray *source, double *weights) override {
		assert("Not implemented" == nullptr);
	}
	void InterpolateTuple(vtkIdType i, vtkIdType id1,
			vtkAbstractArray* source1, vtkIdType id2,
			vtkAbstractArray* source2, double t) override {
		assert("Not implemented" == nullptr);
	}

	vtkVariant GetVariantValue(vtkIdType idx) override {
		assert("Not implemented" == nullptr);
	}

	void SetVariantValue(vtkIdType idx, vtkVariant value) override {
		assert("Not implemented" == nullptr);
	}

	void SetTypedTuple(vtkIdType i, const ValueType *t) override {
		assert("Not implemented" == nullptr);
	}

	void InsertTypedTuple(vtkIdType i, const ValueType *t) override {
		assert("Not implemented" == nullptr);
	}
	vtkIdType InsertNextTypedTuple(const ValueType *t) override {
		assert("Not implemented" == nullptr);
	}

	ValueType GetValue(vtkIdType idx) const override {
//		std::cout << idx << std::endl;
		const auto nodeNumber = idx / 3;
		auto it = mesh->nodes.cbegin();
		std::advance(it, nodeNumber);
		const Node &node = it->second;
		const auto comp = idx - nodeNumber * 3;
		switch (comp) {
			case 0: return node.x;
			case 1: return node.y;
			case 2: return node.z;
		}

		assert("Not implemented" == nullptr);
	}

	ValueType& GetValueReference(vtkIdType idx) override {
		assert("Not implemented" == nullptr);
	}

	void SetValue(vtkIdType idx, ValueType value) override {
		std::cout << idx << ' ' << value << std::endl;
		return;
		assert("Not implemented" == nullptr);
	}

	void GetTypedTuple(vtkIdType idx, ValueType *t) const override {
		assert("Not implemented" == nullptr);
	}

	vtkIdType InsertNextValue(ValueType v) override {
		assert("Not implemented" == nullptr);
	}

	void InsertValue(vtkIdType idx, ValueType v) override {
		assert("Not implemented" == nullptr);
	}
	Mesh *mesh = nullptr;
};
