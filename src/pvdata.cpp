#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <memory>
#include <stdexcept>
#include <algorithm>

enum field_type { scalar_field_type, structure_field_type };

// TODO
// operator==
// no default methods
class field
{
public:
    typedef std::shared_ptr<const field> const_shared_pointer;
    typedef const field* const_pointer;
    //typedef std::shared_ptr<field> shared_pointer;
    //typedef field* pointer;

    virtual ~field() {}
    field_type get_type() const { return m_type; }
    virtual const std::string& get_id() const = 0;
    virtual size_t number_of_bits() const { return 1; }
    
    // TODO
    bool operator==(const field& rhs) { return false; };
    
protected:
    field(field_type type) : m_type(type) {}     
private:
    field_type m_type;
};

enum scalar_type { string_type, int_type, double_type };
const std::string scalar_type_id[] = {
    "string",
    "int",
    "double"
};

 
class scalar :
	public field
{
public:
    static const scalar::const_shared_pointer scalarDoublePtr;

    virtual ~scalar() {}
    scalar_type get_scalar_type() const { return m_scalar_type; }
    virtual const std::string& get_id() const;

    bool operator==(const scalar& rhs) { return (m_scalar_type == rhs.m_scalar_type); };
protected:
    scalar(scalar_type scalar_type) : 
    field(scalar_field_type),
    m_scalar_type(scalar_type) {}     
private:
    scalar_type m_scalar_type;
    friend std::ostream& operator<<(std::ostream &out, const scalar& s);
};

const std::string& scalar::get_id() const
{
    return scalar_type_id[m_scalar_type];
}

const scalar::const_shared_pointer scalar::scalarDoublePtr(new scalar(double_type));

std::ostream& operator<<(std::ostream &out, const scalar& s)
{
    out << s.get_id();
    return out;
}

class structure_member
{
public:
    //typedef std::shared_ptr<structure_member> shared_pointer;
    typedef std::shared_ptr<const structure_member> const_shared_pointer;
    static const const_shared_pointer nullptr_const_shared_pointer;

	structure_member(
		field::const_pointer parent,
		size_t bit_index,
		const std::string& name,
		const field::const_shared_pointer& field) :
		m_parent(parent),
		m_bit_index(bit_index),
		m_name(name),
		m_field(field)
		{
		}
		
	field::const_pointer get_parent() const { return m_parent; }
	size_t get_bit_index() const { return m_bit_index; }
	const std::string& get_name() const { return m_name; }
	const field::const_shared_pointer& get_field() const { return m_field; }
	
	// note: only name and field are compared
    bool operator==(const structure_member& rhs)
    {
    	return (this == &rhs) ||
    	       ((m_name == rhs.m_name) && 
    	  	    (m_field == rhs.m_field));
    };

private:
	field::const_pointer m_parent;
	size_t m_bit_index;
    std::string m_name;
	field::const_shared_pointer m_field;
};

const structure_member::const_shared_pointer structure_member::nullptr_const_shared_pointer;

class structure :
    public field
{
public:
    typedef std::shared_ptr<const structure> const_shared_pointer;

    typedef std::vector<structure_member::const_shared_pointer> members_vector;
    // TODO use of unordered map?
    typedef std::map<std::string, structure_member::const_shared_pointer> members_map;

    virtual ~structure() {}
    virtual const std::string& get_id() const { return m_id; }

	const members_vector& get_members() const { return m_members; }
	
    bool operator==(const structure& rhs)
    {
    	return (this == &rhs) ||
    	       ((m_id == rhs.m_id) && 
    	  	    (m_bits == rhs.m_bits) &&
    		    std::equal(m_members.begin(), m_members.end(), rhs.m_members.begin()));
    };
    
    structure(const std::string& id,
    		  const std::vector<std::string>& names,
              const std::vector<field::const_shared_pointer>& fields) : 
              field(structure_field_type),
              m_id(id)
    {
    	if (names.size() != fields.size())
    		throw std::invalid_argument("names.size() != fields.size()");

    	// the structure itself is at offset 0
		m_bits = 1;  
		  	
    	for (size_t i = 0; i < names.size(); i++)
    	{
    		structure_member::const_shared_pointer member(
    			new structure_member(
    				this,
    				m_bits,
    				names[i],
    				fields[i]
    				)
    			);
    		m_bits += fields[i]->number_of_bits();
    		if (m_members_map.count(names[i]))
    			throw std::invalid_argument("duplicate field name: " + names[i]);
    		m_members_map[names[i]] = member;
    		m_members.push_back(member);
    	}
    }     

    virtual size_t number_of_bits() const { return m_bits; }
    
	//const structure_member::const_shared_pointer& operator[](const std::string& name) const
	const structure_member::const_shared_pointer& operator[](const char* name) const
	{
		members_map::const_iterator iter = m_members_map.find(name);
		if (iter != m_members_map.end())
			return iter->second;
		else
			return structure_member::nullptr_const_shared_pointer;	
	}
	    
	const structure_member::const_shared_pointer& operator[](members_vector::size_type index) const
	{
		return m_members[index];
	}
	
private:
	std::string m_id;
    members_vector m_members;
    members_map m_members_map;
    size_t m_bits;
    
    static const std::string DEFAULT_STRUCTURE_ID;
};

const std::string structure::DEFAULT_STRUCTURE_ID;

// TODO
// operator= (copy)
// operator= (unchecked copy)
// operator= (copy w/ conversion)

// operator== (comparison)

class pvfield
{
public:
    typedef std::shared_ptr<pvfield> shared_pointer;
    static const shared_pointer nullptr_shared_pointer;

	virtual const field::const_shared_pointer& get_field() const = 0;

#define ASSIGNMENT_OPERATORS(T) \
	virtual pvfield& operator=(T value) { throw std::invalid_argument("wrong type"); }; \
	virtual operator T() const { throw std::invalid_argument("wrong type"); };
	
ASSIGNMENT_OPERATORS(double);
//ASSIGNMENT_OPERATORS(float);
//ASSIGNMENT_OPERATORS(int); 	// TODO
//ASSIGNMENT_OPERATORS(std::string);
//ASSIGNMENT_OPERATORS(pvstructure);
//ASSIGNMENT_OPERATORS(pvunion);

#undef ASSIGNMENT_OPERATORS

};

const pvfield::shared_pointer pvfield::nullptr_shared_pointer;

class pvscalar :
	public pvfield
{
public:

	const scalar::const_shared_pointer& get_scalar() const { return m_scalar; } 
	
	virtual const field::const_shared_pointer& get_field() const { return m_scalar; }

	// copy
    pvscalar& operator=(const pvscalar& rhs)
    {
    	if (this != &rhs)
    	{
    		// TODO
    	}
    	return *this;
    }

    bool operator==(const pvscalar& rhs)
    {
	    // TODO
    	return false;
    }
	
protected:
	pvscalar(const scalar::const_shared_pointer& scalar) :
		pvfield(),
		m_scalar(scalar)
	{
	}
private:
	scalar::const_shared_pointer m_scalar;
};

template <typename T>
class pvscalar_value :
	public pvscalar
{
public:
    typedef std::shared_ptr< pvscalar_value<T> > shared_pointer;
    
    typedef T value_type;
    typedef T& value_type_reference;
    typedef T* value_type_pointer;
    typedef const T* value_type_const_pointer;

	pvscalar_value(const scalar::const_shared_pointer& scalar) :
		pvscalar(scalar)
	{
	}
	
	pvscalar_value& operator=(T value) { put(value); return *this; }
	operator T() const { return get(); }
	
	T get() const { return m_value; }
	void put(T value) { m_value = value; }

private:
	T m_value;
};


class pvstructure :
	public pvfield
{
public:

	const structure::const_shared_pointer& get_structure() const { return m_structure; } 
	
	virtual const field::const_shared_pointer& get_field() const { return m_structure; }

    typedef std::vector<pvfield::shared_pointer> members_vector;
    // TODO use of unordered map?
    typedef std::map<std::string, pvfield::shared_pointer> members_map;

//	const pvfield::shared_pointer& operator[](const std::string& name) const
	const pvfield::shared_pointer& operator[](const char* name) const
	{
		members_map::const_iterator iter = m_members_map.find(name);
		if (iter != m_members_map.end())
			return iter->second;
		else
			return pvfield::nullptr_shared_pointer;	
	}

	const pvfield::shared_pointer& operator[](members_vector::size_type index) const
	{
		return m_members[index];
	}

	pvfield& operator,(const std::string& name) const
	{
		return *m_members_map.at(name);
	}
	    
	pvfield& operator,(members_vector::size_type index) const
	{
		return *m_members.at(index);
	}

	pvstructure(const structure::const_shared_pointer& structure_) :
		pvfield(),
		m_structure(structure_)
	{
		const structure::members_vector& members = m_structure->get_members();
		for (
			structure::members_vector::const_iterator iter = members.begin();
			iter != members.end();
			iter++
			)
		{
			const structure_member::const_shared_pointer & sm = *iter;
			
			// TODO
			scalar::const_shared_pointer sf = std::dynamic_pointer_cast<const scalar>(sm->get_field());
			
			pvfield::shared_pointer member(
				new pvscalar_value<double>(sf)
			);
    		m_members_map[sm->get_name()] = member;
    		m_members.push_back(member);
		}		
	}
private:
	structure::const_shared_pointer m_structure;
    members_vector m_members;
    members_map m_members_map;
};


void operator <<=(const pvfield::shared_pointer& lhs, double rhs)
{
	*lhs = rhs;
}

void operator <<=(double& lhs, const pvfield::shared_pointer& rhs)
{
	lhs = *rhs;
}

int main()
{
	/*
	pvscalar_value<double>::shared_pointer fieldDoubleScalar(new pvscalar_value<double>(scalar::scalarDoublePtr));
	*fieldDoubleScalar = 3.12;
	double val = *fieldDoubleScalar;
	
	std::cout << fieldDoubleScalar->get() << std::endl;
	std::cout << val << std::endl;

	pvfield::shared_pointer pvFieldRef = fieldDoubleScalar;
	*pvFieldRef = 4.12;
	// *pvFieldRef = 3;
	
	//int a = *pvFieldRef;
	double d = *pvFieldRef;
*/
	std::vector<std::string> names;
	std::vector<field::const_shared_pointer> fields;
	names.push_back("a"); fields.push_back(scalar::scalarDoublePtr);
	names.push_back("b"); fields.push_back(scalar::scalarDoublePtr);
	
	structure::const_shared_pointer str(new structure("simpleID", names, fields));
//	pvstructure::shared_pointer pvs(new pvstructure(str));
	pvstructure pvs(str);
  
//	*(pvs["a"]) = 3.1;


	//pvs->getSubField<PVDouble>("a")->put(12.3);
	pvs["a"] <<= 12.3;
	*pvs["a"] = 12.4;
	
//	(pvs, "a") = 3.1;
//	(pvs, 2) = 3.2;
	
	double v = *(pvs["a"]);
	double vvv;
	vvv <<= pvs["a"];
	double v2 = (pvs, "a");
  	std::cout << v << std::endl;
  	pvfield::shared_pointer sp = pvs["a"];
  	
  	  
    //std::cout << *(scalar::scalarDoublePtr.get()) << std::endl;  
    return 0;
}
