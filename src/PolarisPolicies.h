#ifndef _POLARISPOLICIES_H_
#define _POLARISPOLICIES_H_

#include <utility>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include "workflow/URIParser.h"
#include "workflow/EndpointParams.h"
#include "workflow/WFNameService.h"
#include "workflow/WFServiceGovernance.h"
#include "src/PolarisConfig.h"

namespace polaris {

/*
struct MatchingString
{
	enum MatchingStringType
	{
		EXACT,
		REGEX,
	};

	MatchingStringType matching_type;

	enum ValueType
	{
		TEXT = 0;
		PARAMETER = 1;
		VARIABLE = 2;
	}

	ValueType value_type;
	std::string value;
};
*/

struct PolarisPolicyConfig
{
	std::string service_name;
	std::string location_zone;
	std::string location_region;
	std::string location_campus;
	bool enable_rule_base_router;
	bool enable_nearby_based_router;

	PolarisPolicyConfig();
};

class PolarisInstanceParams : public PolicyAddrParams
{
public:
	PolarisInstanceParams(struct instance ins,
						  const struct AddressParams *params);

	int get_weight() const { return this->weight; }

private:
	//TODO: remove those not related to select
	std::string id;
	std::string vpc_id;
	int priority;
	int weight;
	bool enable_healthcheck;
	bool healthy;
	bool isolate;
	std::string protocol;
	std::string version;
	std::string logic_set;
	std::string mtime;
	std::string revision;
	std::map<std::string, std::string> metadata;
};

class PolarisPolicy : public WFServiceGovernance
{
public:
	PolarisPolicy(struct PolarisPolicyConfig config);
	virtual ~PolarisPolicy() { }

	int init();
	virtual bool select(const ParsedURI& uri, WFNSTracing *tracing,
						EndpointAddress **addr);

	void update_instances(const std::vector<struct instance>& instances);
	void update_inbounds(const std::vector<struct routing_bound>& inbounds);
	void update_outbounds(const std::vector<struct routing_bound>& outbounds);

private:
	using BoundRulesMap = std::unordered_map<std::string, std::vector<struct routing_bound>>;

	struct PolarisPolicyConfig config;
	BoundRulesMap inbound_rules;
	BoundRulesMap outbound_rules;
	pthread_rwlock_t inbound_rwlock;
	pthread_rwlock_t outbound_rwlock;

	int total_weight;
	int available_weight;

private:
	virtual void recover_one_server(const EndpointAddress *addr);
	virtual void fuse_one_server(const EndpointAddress *addr);
	virtual void add_server_locked(EndpointAddress *addr);
	void clear_instances_locked();

	void get_request_meta(const ParsedURI& uri,
						  std::map<std::string, std::string>& meta);

	bool matching_bounds(const char *caller_service_name,
						 const std::map<std::string, std::string>& meta,
						 std::vector<struct destination_bound> **dst_bounds);

	bool matching_subset(
			std::vector<struct destination_bound> *dest_bounds,
			std::vector<EndpointAddress *>& matched_subset);

	bool matching_rules(
			const std::map<std::string, std::string>& meta,
			const std::vector<struct source_bound>& src_bounds) const;

	bool matching_instances(struct destination_bound *dst_bounds,
							std::vector<EndpointAddress *>& subsets);

	size_t subsets_weighted_random(
			const std::vector<struct destination_bound *>& bounds);

	EndpointAddress *get_one(const std::vector<EndpointAddress *>& instances,
							 WFNSTracing *tracing);
};

}; // namespace polaris

#endif

