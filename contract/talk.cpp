#include <eosio/eosio.hpp>

// Message table
struct [[eosio::table("message"), eosio::contract("talk")]] message {
    uint64_t    id       = {}; // Non-0
    uint64_t    reply_to = {}; // Non-0 if this is a reply
    eosio::name user     = {};
    std::string content  = {};
    std::set<eosio::name>  liked_by   = {};

    uint64_t primary_key() const { return id; }
    uint64_t get_reply_to() const { return reply_to; }
};

using message_table = eosio::multi_index<
    "message"_n, message, eosio::indexed_by<"by.reply.to"_n, eosio::const_mem_fun<message, uint64_t, &message::get_reply_to>>>;

// The contract
class talk : eosio::contract {
  public:
    // Use contract's constructor
    using contract::contract;

    // Post a message
    [[eosio::action]] void post(uint64_t id, uint64_t reply_to, eosio::name user, const std::string& content) {
        message_table table{get_self(), 0};

        // Check user
        require_auth(user);

        // Check reply_to exists
        if (reply_to)
            table.get(reply_to);

        // Create an ID if user didn't specify one
        eosio::check(id < 1'000'000'000ull, "user-specified id is too big");
        if (!id)
            id = std::max(table.available_primary_key(), 1'000'000'000ull);

        // Record the message
        table.emplace(get_self(), [&](auto& message) {
            message.id       = id;
            message.reply_to = reply_to;
            message.user     = user;
            message.content  = content;
        });
    }
    
    //like a post/message
    [[eosio::action]] void like(uint64_t id, eosio::name by_user) {

        message_table table{get_self(), 0};

        // Check user
        require_auth(by_user);

        auto it = table.find(id);
        //check if its valid post to like it.
        if ( it != std::end(table)){
            // Ideally we need to -- throw std::logic_error("Invalid post id :"+std::to_string(id)); if no post xists with id
            std::set<eosio::name> liked_by = it->liked_by;
            if( liked_by.find(by_user) == std::end(liked_by)) {
                //Ideally we need to throw excetion if same user trying to like several times to the same post.
               // throw std::logic_error( by_user.to_string() +": user can not like a post more than once");
                liked_by.emplace(by_user);
            }
         }
    }

};
