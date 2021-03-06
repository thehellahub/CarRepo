
// Copyright 2020 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GRPC_CORE_LIB_SECURITY_AUTHORIZATION_AUTHORIZATION_ENGINE_H
#define GRPC_CORE_LIB_SECURITY_AUTHORIZATION_AUTHORIZATION_ENGINE_H

#include <grpc/support/port_platform.h>

#include <grpc/support/log.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "absl/container/flat_hash_set.h"
#if COCOAPODS==1
  #include  "src/core/ext/upb-generated/envoy/config/rbac/v3/rbac.upb.h"
#else
  #include  "envoy/config/rbac/v3/rbac.upb.h"
#endif
#if COCOAPODS==1
  #include  "src/core/ext/upb-generated/google/api/expr/v1alpha1/syntax.upb.h"
#else
  #include  "google/api/expr/v1alpha1/syntax.upb.h"
#endif
#if COCOAPODS==1
  #include  "third_party/upb/upb/upb.hpp"
#else
  #include  "upb/upb.hpp"
#endif

#include "src/core/lib/security/authorization/evaluate_args.h"
#include "src/core/lib/security/authorization/mock_cel/activation.h"

namespace grpc_core {

// AuthorizationEngine makes an AuthorizationDecision to ALLOW or DENY the
// current action based on the condition fields in provided RBAC policies.
// The engine may be constructed with one or two policies. If two polcies,
// the first policy is deny-if-matched and the second is allow-if-matched.
// The engine returns UNDECIDED decision if it fails to find a match in any
// policy. This engine ignores the principal and permission fields in RBAC
// policies. It is the caller's responsibility to provide RBAC policies that
// are compatible with this engine.
//
// Example:
// AuthorizationEngine*
// auth_engine = AuthorizationEngine::CreateAuthorizationEngine(rbac_policies);
// auth_engine->Evaluate(evaluate_args); // returns authorization decision.
class AuthorizationEngine {
 public:
  // rbac_policies must be a vector containing either a single policy of any
  // kind, or one deny policy and one allow policy, in that order.
  static std::unique_ptr<AuthorizationEngine> CreateAuthorizationEngine(
      const std::vector<envoy_config_rbac_v3_RBAC*>& rbac_policies);

  // Users should use the CreateAuthorizationEngine factory function
  // instead of calling the AuthorizationEngine constructor directly.
  explicit AuthorizationEngine(
      const std::vector<envoy_config_rbac_v3_RBAC*>& rbac_policies);
  // TODO(mywang@google.com): add an Evaluate member function.

 private:
  enum Action {
    kAllow,
    kDeny,
  };

  std::unique_ptr<mock_cel::Activation> CreateActivation(
      const EvaluateArgs& args);

  std::map<const std::string, const google_api_expr_v1alpha1_Expr*>
      deny_if_matched_;
  std::map<const std::string, const google_api_expr_v1alpha1_Expr*>
      allow_if_matched_;
  upb::Arena arena_;
  absl::flat_hash_set<std::string> envoy_attributes_;
  absl::flat_hash_set<std::string> header_keys_;
  std::unique_ptr<mock_cel::CelMap> headers_;
};

}  // namespace grpc_core

#endif /* GRPC_CORE_LIB_SECURITY_AUTHORIZATION_AUTHORIZATION_ENGINE_H */
