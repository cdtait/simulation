from __future__ import print_function
import md_generation
import sys

def show_error_stats(generator):
    print("Error generation",file=sys.stderr)
    print("Corrupt event:"+str(generator.error_strategy.corruption_error),file=sys.stderr)
    print("Duplicate order id:"+str(generator.error_strategy.dup_error),file=sys.stderr)
    print("No order matching trade:"+str(generator.error_strategy.best_sell_le_best_buy_error),file=sys.stderr)
    print("No order id with cancel or modify:"+str(generator.error_strategy.no_order_for_modify),file=sys.stderr)
    print("No matching trade with order:"+str(generator.error_strategy.no_order_for_trade_error),file=sys.stderr)
    print("Order range:"+str(generator.error_strategy.order_range_error),file=sys.stderr)
    print("Order syntax:"+str(generator.error_strategy.order_parse_error),file=sys.stderr)
    print("Side error:"+str(generator.error_strategy.side_error),file=sys.stderr)
    print("Quantity range:"+str(generator.error_strategy.quantity_range_error),file=sys.stderr)
    print("Quantity syntax:"+str(generator.error_strategy.quantity_parse_error),file=sys.stderr)
    print("Price range:"+str(generator.error_strategy.price_range_error),file=sys.stderr)
    print("Price syntax:"+str(generator.error_strategy.price_parse_error),file=sys.stderr)
    
if __name__ == '__main__':
    from optparse import OptionParser
    parser = OptionParser()
    parser.add_option("-c", "--num_changes",
                  action="store", type="int", dest="changes", default=2000)
    parser.add_option("-e", "--errors",
                  action="store_true", dest="errors")
    parser.add_option("-s", "--strategy",
                  action="store", type="string", dest="strategy", default="RandomStrategy")
    (options, args) = parser.parse_args()
    generator = md_generation.MDGenerator(options.errors,options.strategy)
    generator.create_order_book()
    generator.generate_book_events(options.changes)
    show_error_stats(generator)
    