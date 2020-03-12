using namespace std;
using namespace eosio;
#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/asset.hpp>

namespace eosio {

   using std::string;

   class [[eosio::contract("eosio.token")]] token : public contract {
      public:
         using contract::contract;
         //@abi action
         [[eosio::action]]
         void create( name   brokerage,
                      asset  maximum_supply);

         [[eosio::action]]
         void issue( name to, asset quantity, string order_id );
      
         [[eosio::action]]
         void transfer( name    from,
                        name    to,
                        asset   quantity,
                        string  order_id );

      
         static asset get_supply( name token_contract_account, symbol_code sym_code )
         {
            stats statstable( token_contract_account, sym_code.raw() );
            const auto& st = statstable.get( sym_code.raw() );
            return st.supply;
         }

         static asset get_balance( name token_contract_account, name owner, symbol_code sym_code )
         {
            accounts accountstable( token_contract_account, owner.amount );
            const auto& ac = accountstable.get( sym_code.raw() );
            print(ac.balance);
            return ac.balance;
         }
      //@abi tables
      private: //read, update, create, delete functions
         struct [[eosio::table]] currency_stats {
            name     brokerage; //name
            asset    supply;
            asset    max_supply;
            
            uint64_t primary_key()const { return supply.symbol.code().raw(); }
         };

         struct [[eosio::table]] account {
            asset    balance; 
            uint64_t primary_key()const { return balance.symbol.code().raw(); }
         };

         typedef eosio::multi_index< "accounts"_n, account > accounts;
         typedef eosio::multi_index< "stat"_n, currency_stats > stats;

         void sub_balance( name owner, asset amount );
         void add_balance( name owner, asset amount, name ram_payer );
   };
}
namespace eosiosystem {
   class system_contract;
}
