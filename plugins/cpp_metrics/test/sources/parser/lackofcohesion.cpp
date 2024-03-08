////////////////

struct trivial_0_0
{

};

////////////////

struct trivial_1_0
{
	int field1;
};

struct trivial_2_0
{
	int field1;
	int field2;
};

struct trivial_3_0
{
	int field1;
	int field2;
	int field3;
};

////////////////

struct trivial_0_1
{
	void method1() {}
};

struct trivial_0_2
{
	void method1() {}
	void method2() {}
};

struct trivial_0_3
{
	void method1() {}
	void method2() {}
	void method3() {}
};

////////////////

struct fully_cohesive_read
{
	int single_field;
	
	int single_method() const { return single_field; }
};

struct fully_cohesive_write
{
	int single_field;
	
	void single_method(int value) { single_field = value; }
};

struct fully_cohesive_read_write
{
	int single_field;
	
	bool single_method(int value)
	{
		if (single_field != value)
		{
			single_field = value;
			return true;
		}
		else return false;
	}
};

////////////////

struct fully_cohesive_2_1
{
	int field1;
	int field2;
	
	int method1() const { return field1 + field2; }
};

struct fully_cohesive_1_2
{
	int field1;
	
	int method1() const { return +field1; }
	int method2() const { return -field1; }
};

struct fully_cohesive_2_2
{
	int field1;
	int field2;
	
	int method1() const { return field1 + field2; }
	int method2() const { return field1 - field2; }
};

struct fully_cohesive_3_3
{
	int field1;
	int field2;
	int field3;
	
	int method1() const { return field1 + field2 + field3; }
	int method2() const { return field1 * field2 * field3; }
	void method3(int value) { field1 = field2 = field3 = value; }
};

////////////////

struct fully_incohesive_1_1
{
	int field1;
	
	void method1() {}
};

struct fully_incohesive_1_2
{
	int field1;
	
	void method1() {}
	void method2() {}
};

struct fully_incohesive_2_1
{
	int field1;
	int field2;
	
	void method1() {}
};

struct fully_incohesive_2_2
{
	int field1;
	int field2;
	
	void method1() {}
	void method2() {}
};

////////////////

struct partially_cohesive_1_2
{
	int field1;
	
	int method1() const { return field1; }
	int method2() const { return 42; }
};

struct partially_cohesive_2_1
{
	int field1;
	int field2;
	
	int method1() const { return field1 + 42; }
};

struct partially_cohesive_2_2_A
{
	int field1;
	int field2;
	
	int method1() const { return field1 + 42; }
	int method2() const { return 42; }
};

struct partially_cohesive_2_2_B
{
	int field1;
	int field2;
	
	int method1() const { return field1 + 42; }
	int method2() const { return field2 - 42; }
};

struct partially_cohesive_2_2_C
{
	int field1;
	int field2;
	
	int method1() const { return field1 + 42; }
	int method2() const { return field2 - field1; }
};

////////////////

struct same_partial_coh_A
{
	int field1, field2, field3;
	
	int method1() const { return field1; }
	void method2() { field2 = field1; }
	int method3() { return field3 = field2 * field1; }
};

struct same_partial_coh_B
{
	int field1, field2, field3;
	
	int method1() const { return field1 + field1; }
	void method2() { field2 += field1; }
	int method3() { return field3 = field2 * field2 + field1 * field1; }
};

struct same_partial_coh_C
{
	int field1, field2, field3;
	
	int method1() const { return 2 * field3; }
	void method2() { field2 = 42 * field3 / field2; }
	int method3() { return field1 = field3 + field2; }
};
