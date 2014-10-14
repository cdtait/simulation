'''
Created on Oct 08, 2014

@author: ctait
'''
from patterns import  Singleton

@Singleton
class OrderBookStats(object):
    def __init__(self):
        self.corrupt_cnt=0
        self.dup_orderid_cnt=0
        self.no_order_for_trade_cnt=0
        self.no_order_for_mod_cnt=0
        self.best_sell_le_best_buy_cnt=0
        self.bad_order_values_cnt=0
        self.missing_price_level_cnt=0;
        
    def event_error(self):
        self.corrupt_cnt+=1
    
    def corrupt_error(self):
        self.corrupt_cnt+=1
         
    def dup_orderid(self):
        self.dup_orderid_cnt+=1
            
    def no_order_for_trade(self):
        self.no_order_for_trade_cnt+=1
        
    def bad_order_value(self):
        self.bad_order_values_cnt+=1
                
    def no_order_for_modify(self):
        self.no_order_for_mod_cnt+=1
        
    def missing_price_level(self):
        self.missing_price_level_cnt+=1      
        
    def best_sell_le_best_buy(self):
        self.best_sell_le_best_buy_cnt+=1 
        
    def __str__(self):
        s='Error summary\ncorrupt:{0:3d}\ndup_orderid:{1:3d}\n'\
        'no_order_for_trade:{2:3d}\nno_order_for_mod:{3:3d}\n'\
        'best_sell_le_best_buy:{4:3d}\nbad_order_values:{5:3d}\n'\
        'missing_price_level_cnt:{6:3d}\n'.format(
        self.corrupt_cnt,
        self.dup_orderid_cnt,
        self.no_order_for_trade_cnt,
        self.no_order_for_mod_cnt,
        self.best_sell_le_best_buy_cnt,
        self.bad_order_values_cnt,
        self.missing_price_level_cnt)
        
        return s
    
stats=OrderBookStats

def get_event(message,stats):
    event=message[0]
    separator=message[1]
    #

    if event not in ('A','M','X','T'):
        #raise ValueError("event is not (A,M,X,T) found : %r" % event)
        stats.event_error()
        return ('','')
        
    if separator!=',':
        #raise ValueError("separator missing after event code, found %r" % separator)
        stats.corrupt_error()
        return ('','')
    #
    return (event,message[2:])
#
def get_order(message,stats):
    """
    Extracts the properties from the message to create an order object
    """
    sp=message.split(',')
    spl=len(sp)
    #
    if spl!=4:
        #raise ValueError("expected order to made of 4 components found: %r" % spl)
        stats.corrupt_error()
        return (0,'',0,0.0)
    #
    (orderid,side,quantity,price)=(int(sp[0]),sp[1],int(sp[2]),float(sp[3]))
    #
    if  orderid <= 0:
        #raise ValueError("orderid is not valid positive integer: %r" % orderid)
        stats.bad_order_value()
        return (0,'',0,0.0)
    if  side not in ('S','B'):
        #raise ValueError("side is not ('S','B') found : %r" % side)
        stats.corrupt_error()
        return (0,'',0,0.0)
    if  quantity <= 0:
        #raise ValueError("Quantity is not valid positive integer: %r" % quantity)
        stats.bad_order_value()
        return (0,'',0,0.0)
    if  price <= 0:
        #raise ValueError("Price is not valid float: %r" % price)
        stats.bad_order_value()
        return (0,'',0,0.0)
    #
    return (orderid,side,quantity,price)

def get_trade(message,stats):
    """
    Extracts the properties from the message to create an trade object
    """
    sp=message.split(',')
    spl=len(sp)
    #
    if spl!=3:
        #raise ValueError("expected order to made of 4 components found: %r" % spl)
        stats.corrupt_error()
        return (0,'',0,0.0)
    #
    (side,quantity,price)=(sp[0],int(sp[1]),float(sp[1]))
    
    if  side not in ('S','B'):
        #raise ValueError("side is not ('S','B') found : %r" % side)
        stats.corrupt_error()
        return (0,'',0,0.0)
    if  quantity <= 0:
        #raise ValueError("quantity is not valid positive integer: %r" % quantity)
        stats.bad_order_value()
        return (0,0.0)
    if  price <= 0:
        #raise ValueError("price is not valid float: %r" % price)
        stats.bad_order_value()
        return (0,0.0)

    return (side,quantity,price)

class OrderBook(object):
    '''
    OrderBook prototype in python implemtnts the features
    of the specification for add,modify,cancel and trade
    Uses an unordered associative map as underlying data
    structure
    '''
    mid_str='\nMid price {0:5.2f}'
    
    header='\n|-----------------------------------------|\n' \
             '|         Bid        |         Ask        |\n' \
             '|-----------------------------------------|\n' \
             '|Level| Num | Qty |Price|Price| Qty | Num |\n' \
             '|-----------------------------------------|\n'

    trailer= '|-----------------------------------------|\n'
    
    level_format='|{6:5d}|{0:5d}|{1:5d}|{2:5.0f}|{3:5.0f}|{4:5d}|{5:5d}|\n'
    
    @staticmethod
    def to_string(bl,sl):
        """
        Transform to a string output for readable format
        """
        bls = sorted(bl, key=lambda t: t[0], reverse=True)
        sls = sorted(sl, key=lambda t: t[0])
                    
        nl = max(len(bls),len(sls))
        (bdcontr,bdquantity,bdprice,sdprice,sdquantity,sdcontr)=([0,]*nl,[0,]*nl,[0.0,]*nl,[0.0,]*nl,[0,]*nl,[0,]*nl)

        levs=''
        len_bls=len(bls)
        len_sls=len(sls)
        for i in range(nl):
            if len_bls > i:
                (bdprice[i], buyLevel) = bls[i]
                bdcontr[i]=len(buyLevel)
                bdquantity[i]=sum(buyLevel.values())
            if len_sls > i:
                (sdprice[i], sellLevel) = sls[i]
                sdcontr[i]=len(sellLevel)
                sdquantity[i]=sum(sellLevel.values())
            levs+=OrderBook.level_format.format(bdcontr[i],bdquantity[i],bdprice[i],sdprice[i],sdquantity[i],sdcontr[i],i)

        mid_price=0.0
        if bdprice[0] > 0 and sdprice[0] > 0:
            mid_price = (bdprice[0]+sdprice[0])/2.0
        
        return OrderBook.mid_str.format(mid_price) + OrderBook.header + levs + OrderBook.trailer
    
    def get_mid(self):
        """
        Return the mid price
        """
        buy_prices=self.buyLevels.keys()
        sell_prices=self.sellLevels.keys()
        if len(buy_prices) > 0 and len(sell_prices):
            best_buy = max(buy_prices)
            best_sell = min(sell_prices)
            return (best_sell+best_buy)/2.0
        else:
            0.0
            
    def __init__(self):
        '''
        Constructor
        '''
        self.buyLevels={}
        self.sellLevels={}
        self.trades={}
        self.matchedOrderPendingTrades=()
        self.stats=OrderBookStats()

    def add(self, message):
        """
        Add the order to the order book from the message extraction
        """
        (orderid,side,quantity,price)=get_order(message,self.stats)
        if side == "S":
            self.add_at_price_level(self.sellLevels,price,orderid,quantity)
            self.monitor_sell_order_match(orderid,quantity,price)
        elif side == "B":
            self.add_at_price_level(self.buyLevels,price,orderid,quantity)
            self.monitor_buy_order_match(orderid,quantity,price)

    def modify(self, message):
        """
        Modify the order to the order book from the message extraction
        """
        (orderid,side,quantity,price)=get_order(message,self.stats)
        if side == "S":
            self.modify_at_price_level(self.sellLevels,price,orderid,quantity)
        elif side == "B":
            self.modify_at_price_level(self.buyLevels,price,orderid,quantity)

    def cancel(self, message):
        """
        Cancel the order to the order book from the message extraction
        """
        (orderid,side,quantity,price)=get_order(message,self.stats)
        if side == "S":
            self.cancel_at_price_level(self.sellLevels,price,orderid,quantity)
        elif side == "B":
            self.cancel_at_price_level(self.buyLevels,price,orderid,quantity)

    def add_at_price_level(self,levels,price,orderid,quantity):
        if price in levels:
            levels[price][orderid]=quantity
        else:
            levels[price] = {orderid : quantity}
                
    def modify_at_price_level(self,levels,price,orderid,quantity):
        if price in levels:
            if orderid in levels[price]:
                levels[price][orderid]=quantity
            else:
                self.stats.no_order_for_modify()
        else:
            self.stats.missing_price_level()

    def cancel_at_price_level(self,levels,price,orderid,quantity):
        if price in levels:
            if orderid in levels[price]:
                if levels[price][orderid] <= quantity:
                    del levels[price][orderid]
                else:
                    levels[price][orderid] -= quantity
            else:
                self.stats.no_order_for_modify()

            if len(levels[price])==0:
                del levels[price]
        else:
            self.stats.missing_price_level()
                        
    def trade(self, message):
        """
        Add trade to the order book from the message extraction
        """
        (side,quantity,price)=get_trade(message,self.stats)
        if quantity>0:
            if price in self.trades:
                self.trades[price].append(quantity)
            else:
                self.trades[price]=[quantity]

            self.monitor_trade(quantity,price)

    def get_top_bid(self):
        return max(self.buyLevels.keys())
    
    def get_top_ask(self):
        return min(self.sellLevels.keys())
           
    def get_bottom_bid(self):
        return min(self.buyLevels.keys())
    
    def get_bottom_ask(self):
        return max(self.sellLevels.keys())
    
    def monitor_buy_order_match(self,orderid,quantity,price):
        if len(self.matchedOrderPendingTrades)>0:
            self.stats.best_sell_le_best_buy()
        
        if len(self.sellLevels) > 0:
            lowestSellPrice=min(self.sellLevels.keys())
            if lowestSellPrice <= price:
                self.matchedOrderPendingTrades=(price,orderid)

    def monitor_sell_order_match(self,orderid,quantity,price):
        if len(self.matchedOrderPendingTrades)>0:
            self.stats.best_sell_le_best_buy()
        
        if len(self.buyLevels) > 0:
            lowestBuyPrice=max(self.buyLevels.keys())
            if lowestBuyPrice > price:
                self.matchedOrderPendingTrades=(price,orderid)   
                
    def monitor_trade(self,quantity,price):
        if len(self.matchedOrderPendingTrades) > 0 and self.matchedOrderPendingTrades[0]>=price:
            self.matchedOrderPendingTrades=()
            